#include "dfs/util/dfs.h"
#include "dfs/util/parameter.h"
#include "base/common/ErrorMsg.h"
#include "dfs/util/func.h"
#include "base/fs/FsName.h"
#include "file_repair.h"
#include "dataservice.h"


namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;
using namespace neptune::dfs;

FileRepair::FileRepair() :
  init_status_(false), dataserver_id_(0), dfs_client_(NULL)
{
  src_addr_[0] = '\0';
}

FileRepair::~FileRepair()
{
}

bool FileRepair::init(const uint64_t dataserver_id)
{
  if (init_status_)
  {
    return true;
  }

  //LOG(INFO, "file repair init ns address: %s:%d",
  //          SYSPARAM_DATASERVER.local_ns_ip_.length() > 0 ? SYSPARAM_DATASERVER.local_ns_ip_.c_str() : "none",
  //          SYSPARAM_DATASERVER.local_ns_port_);

  int ret = false;
  if (SYSPARAM_DATASERVER.local_ns_ip_.length() > 0 &&
      SYSPARAM_DATASERVER.local_ns_port_ > 0)
  {
    snprintf(src_addr_, MAX_ADDRESS_LENGTH, "%s:%d",
              SYSPARAM_DATASERVER.local_ns_ip_.c_str(),
              SYSPARAM_DATASERVER.local_ns_port_);

    dataserver_id_ = dataserver_id;
    init_status_ = true;

    dfs_client_ = DfsClientImpl::Instance();
    ret =
      dfs_client_->initialize(NULL, DEFAULT_BLOCK_CACHE_TIME, DEFAULT_BLOCK_CACHE_ITEMS, false) == SUCCESS ?
      true : false;
    if (ret)
    {
      init_status_ = true;
    }
  }

  return ret;
}

int FileRepair::fetch_file(const CrcCheckFile& crc_check_record, char* tmp_file)
{
  if (NULL == tmp_file || !init_status_)
  {
    return ERROR;
  }

  int ret = SUCCESS;
  FSName fsname(crc_check_record.block_id_, crc_check_record.file_id_);
  int src_fd = dfs_client_->open(fsname.get_name(), NULL, src_addr_, T_READ);

  if (src_fd <= 0)
  {
    //LOG(ERROR, "%s open src DfsFile read fail: %u %"PRI64_PREFIX"u, ret: %d",
    //          fsname.get_name(), crc_check_record.block_id_,
    //          crc_check_record.file_id_, src_fd);
    ret = ERROR;
  }
  else
  {
    DfsFileStat file_stat;
    if ((ret = dfs_client_->fstat(src_fd, &file_stat)) != SUCCESS)
    {
      //LOG(ERROR, "%s stat DfsFile fail. blockid: %u, fileid: %"PRI64_PREFIX"u, ret: %d",
      //          fsname.get_name(), crc_check_record.block_id_, crc_check_record.file_id_, ret);
    }
    else
    {
      int fd = -1;
      get_tmp_file_name(tmp_file, dynamic_cast<DataService*>(DataService::instance())->get_real_work_dir().c_str(), fsname.get_name());

      if ((fd = open(tmp_file, O_WRONLY | O_CREAT | O_TRUNC, 0660)) == -1)
      {
      //  LOG(ERROR, "copy file, open file: %s failed. error: %s", tmp_file, strerror(errno));
        ret = ERROR;
      }
      else
      {
        char data[MAX_READ_SIZE];
        int32_t rlen = 0;
        int32_t wlen = 0;
        int32_t offset = 0;
        uint32_t crc = 0;

        ret = ERROR;
        while (1)
        {
          if ((rlen = dfs_client_->read(src_fd, data, MAX_READ_SIZE)) <= 0)
          {
            //LOG(ERROR, "%s read src DfsFile fail. blockid: %u, fileid: %"PRI64_PREFIX"u, ret: %d",
            //          fsname.get_name(), crc_check_record.block_id_, crc_check_record.file_id_, rlen);
            break;
          }

          if ((wlen = write_file(fd, data, rlen)) != rlen)
          {
            //LOG(ERROR, "%s write file fail: blockid: %u, fileid: %"PRI64_PREFIX"u, write len: %d, ret: %d",
            //          fsname.get_name(), crc_check_record.block_id_, crc_check_record.file_id_, rlen, wlen);
              break;
          }

          crc = Func::crc(crc, data, rlen);
          offset += rlen;

          if (rlen < MAX_READ_SIZE || offset >= file_stat.size_)
          {
            ret = SUCCESS;
            break;
          }
        }

        if (SUCCESS == ret)
        {
          if (crc != file_stat.crc_ || crc != crc_check_record.crc_ || offset != file_stat.size_)
          {
            //LOG(ERROR,
            //          "file %s crc error. blockid: %u, fileid: %"PRI64_PREFIX"u, %u <> %u, checkfile crc: %u, size: %d <> %"PRI64_PREFIX"d",
            //          fsname.get_name(), crc_check_record.block_id_, crc_check_record.file_id_,
            //          crc, file_stat.crc_, crc_check_record.crc_, offset, file_stat.size_);
            ret = ERROR;
          }
        }

        ::close(fd);
      }
    }

    dfs_client_->close(src_fd);
  }

  return ret;
}

int FileRepair::repair_file(const CrcCheckFile& crc_check_record, const char* tmp_file)
{
  if (NULL == tmp_file || !init_status_)
  {
    return ERROR;
  }

  FSName fsname(crc_check_record.block_id_, crc_check_record.file_id_);
  int ret = dfs_client_->save_file_update(tmp_file, T_DEFAULT, fsname.get_name(), NULL, src_addr_) < 0 ? ERROR : SUCCESS;
  // int ret = dfs_client_->save_file(tmp_file, fsname.get_name()) < 0 ? ERROR : SUCCESS;

  if (SUCCESS != ret)
  {
    //LOG(ERROR, "%s repair file fail, blockid: %u fileid: %" PRI64_PREFIX "u, ret: %d",
    //          fsname.get_name(), crc_check_record.block_id_, crc_check_record.file_id_, ret);
  }
  else
  {
    //LOG(INFO, "%s repair file success, blockid: %u fileid: %" PRI64_PREFIX "u",
    //          fsname.get_name(), crc_check_record.block_id_, crc_check_record.file_id_);
  }

  return ret;
}

void FileRepair::get_tmp_file_name(char* buffer, const char* path, const char* name)
{
  if (NULL == buffer || NULL == path || NULL == name )
  {
    return;
  }
  snprintf(buffer, MAX_PATH_LENGTH, "%s/tmp/%s", path, name);
}

int FileRepair::write_file(const int fd, const char* buffer, const int32_t length)
{
  int32_t bytes_write = 0;
  int wlen = 0;
  while (bytes_write < length)
  {
    wlen = write(fd, buffer + bytes_write, length - bytes_write);
    if (wlen < 0)
    {
      //LOG(ERROR, "file repair failed when write. error desc: %s\n", strerror(errno));
      bytes_write = wlen;
      break;
    }
    else
    {
      bytes_write += wlen;
    }
  }

  return bytes_write;
}

}
}
}
