#include "mmap_file_op.h"
#include "base/common/ErrorMsg.h"
#include "dfs/util/func.h"
#include "dfs/util/dfs.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;

int MMapFileOperation::mmap_file(const MMapOption& mmap_option)
{
  if (mmap_option.max_mmap_size_ < mmap_option.first_mmap_size_)
  {
    return ERROR;
  }

  if (0 == mmap_option.max_mmap_size_)
  {
    return ERROR;
  }

  int fd = check_file();
  if (fd < 0)
    return fd;

  if (!is_mapped_)
  {
    gDelete(map_file_);
    map_file_ = new MMapFile(mmap_option, fd);
    is_mapped_ = map_file_->map_file(true);
  }

  if (is_mapped_)
  {
    return SUCCESS;
  }
  else
  {
    return ERROR;
  }
}

int MMapFileOperation::munmap_file()
{
  if (is_mapped_ && NULL != map_file_)
  {
    gDelete(map_file_);
    is_mapped_ = false;
  }
  return SUCCESS;
}

int MMapFileOperation::rename_file()
{
  int ret = ERROR;
  if (is_mapped_ && NULL != map_file_)
  {
    std::string new_file_name = file_name_;
    new_file_name += "." + Func::time_to_str(time(NULL), 1);
    ret = ::rename(file_name_, new_file_name.c_str());
    free(file_name_);
    file_name_ = strdup(new_file_name.c_str());
  }
  return ret;
}

void* MMapFileOperation::get_map_data() const
{
  if (is_mapped_)
    return map_file_->get_data();

  return NULL;
}

int MMapFileOperation::pread_file(char* buf, const int32_t size, const int64_t offset)
{
  if (is_mapped_ && (offset + size) > map_file_->get_size())
  {
    //LOG(DEBUG, "mmap file pread, size: %d, offset: %" PRI64_PREFIX "d, map file size: %d. need remap",
    //    size, offset, map_file_->get_size());
    map_file_->remap_file();
  }

  if (is_mapped_ && (offset + size) < map_file_->get_size())
  {
    memcpy(buf, (char *) map_file_->get_data() + offset, size);
    return SUCCESS;
  }

  return FileOperation::pread_file(buf, size, offset);
}

int MMapFileOperation::pread_file(ParaInfo& para_info, const int32_t size, const int64_t offset)
{
  if (is_mapped_ && (offset + size) > map_file_->get_size())
  {
    map_file_->remap_file();
  }

  if (is_mapped_ && (offset + size) <= map_file_->get_size())
  {
    para_info.set_self_buf((char*) map_file_->get_data() + offset);
    return SUCCESS;
  }

  return FileOperation::pread_file(para_info.get_new_buf(), size, offset);
}

int MMapFileOperation::pwrite_file(const char* buf, const int32_t size, const int64_t offset)
{
  if (is_mapped_ && (offset + size) > map_file_->get_size())
  {
    //LOG(DEBUG, "mmap file write, size: %d, offset: %" PRI64_PREFIX "d, map file size: %d, need remap",
    //    size, offset, map_file_->get_size());
    map_file_->remap_file();
  }

  if (is_mapped_ && (offset + size) <= map_file_->get_size())
  {
    memcpy((char *) map_file_->get_data() + offset, buf, size);
    return SUCCESS;
  }

  return FileOperation::pwrite_file(buf, size, offset);
}

int MMapFileOperation::flush_file()
{
  if (is_mapped_)
  {
    if (map_file_->sync_file())
    {
      return SUCCESS;
    }
    else
    {
      return ERROR;
    }
  }
  return FileOperation::flush_file();
}

}
}
}
