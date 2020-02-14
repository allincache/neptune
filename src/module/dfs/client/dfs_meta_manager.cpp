#include "dfs_meta_manager.h"
#include "base/fs/FsName.h"

using namespace neptune::dfs;
using namespace std;

int DfsMetaManager::initialize()
{
  int ret = SUCCESS;
  if ((ret = DfsClientImpl::Instance()->initialize(NULL, DEFAULT_BLOCK_CACHE_TIME, 1000, false)) != SUCCESS)
  {
    //LOG(ERROR, "initialize dfs client failed, ret: %d", ret);
  }
  return ret;
}

int DfsMetaManager::destroy()
{
  return DfsClientImpl::Instance()->destroy();
}

int32_t DfsMetaManager::get_cluster_id(const char* ns_addr)
{
  return DfsClientImpl::Instance()->get_cluster_id(ns_addr);
}

int64_t DfsMetaManager::read_data(const char* ns_addr, const uint32_t block_id, const uint64_t file_id,
    void* buffer, const int32_t offset, const int64_t length)
{
  FSName fsname;
  fsname.set_block_id(block_id);
  fsname.set_file_id(file_id);

  int64_t ret_length = -1;
  int fd = DfsClientImpl::Instance()->open(fsname.get_name(), NULL, ns_addr, T_READ);
  if (fd < 0)
  {
    //LOG(ERROR, "open read file error, file_name: %s, fd: %d", fsname.get_name(), fd);
  }
  else
  {
    DfsClientImpl::Instance()->lseek(fd, offset, T_SEEK_SET);
    ret_length = DfsClientImpl::Instance()->read(fd, buffer, length);
    DfsClientImpl::Instance()->close(fd);
  }

  return ret_length;
}

int64_t DfsMetaManager::write_data(const char* ns_addr, const void* buffer, const int64_t offset, const int64_t length,
    FragMeta& frag_meta)
{
  //TODO get dfs_file from file_pool

  int ret = SUCCESS;
  int64_t cur_pos = 0;
  int64_t cur_length, left_length = length;
  char dfsname[MAX_FILE_NAME_LEN];
  int fd = DfsClientImpl::Instance()->open(NULL, NULL, ns_addr, T_WRITE);
  if (fd < 0)
  {
    //LOG(ERROR, "open write file error, fd: %d", fd);
  }
  else
  {
    int64_t write_length = 0;
    do
    {
      cur_length = min(left_length, MAX_WRITE_DATA_IO);
      write_length = DfsClientImpl::Instance()->write(fd, reinterpret_cast<const char*>(buffer) + cur_pos, cur_length);
      if (write_length < 0)
      {
        //LOG(ERROR, "dfs write data error, ret: %"PRI64_PREFIX"d", write_length);
        ret = ERROR;
        break;
      }
      cur_pos += write_length;
      left_length -= write_length;
    }
    while(left_length > 0);
    DfsClientImpl::Instance()->close(fd, dfsname, MAX_FILE_NAME_LEN);
    //LOG(DEBUG, "dfs write success, dfsname: %s", dfsname);

    FSName fsname;
    fsname.set_name(dfsname);

    frag_meta.block_id_ = fsname.get_block_id();
    frag_meta.file_id_ = fsname.get_file_id();
    frag_meta.offset_ = offset;
    frag_meta.size_ = length - left_length;
  }

  return (length - left_length);
}
