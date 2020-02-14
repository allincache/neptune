#ifndef N_DFS_DS_BLOCKFILEFORMATER_H
#define N_DFS_DS_BLOCKFILEFORMATER_H

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "base/common/ErrorMsg.h"
#include "dataserver_define.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class FileFormater
{
 public:
  FileFormater()
  {
  }

  virtual ~FileFormater()
  {
  }

 public:
  virtual int block_file_format(const int fd, const int64_t size) = 0;
};

class Ext4FileFormater: public FileFormater
{
public:
  Ext4FileFormater()
  {
  }
  ~Ext4FileFormater()
  {
  }
  int block_file_format(const int file_fd, const int64_t file_size);

};

int Ext4FileFormater::block_file_format(const int file_fd, const int64_t file_size)
{
  if (file_fd < 0)
  {
    return file_fd;
  }

  if (file_size <= 0)
  {
    return file_size;
  }

  int ret = SUCCESS;
#if DFS_DS_FALLOCATE
  ret = fallocate(file_fd, 0, 0, file_size);
#else
#ifdef __NR_fallocate
  ret = syscall(__NR_fallocate, file_fd, 0, 0, file_size);
#else
  ret = EXIT_FALLOCATE_NOT_IMPLEMENT;
#endif
#endif

  return ret;
}

class Ext3FullFileFormater: public FileFormater
{
public:
  Ext3FullFileFormater()
  {
  }
  ~Ext3FullFileFormater()
  {
  }
  int block_file_format(const int file_fd, const int64_t file_size);

};

int Ext3FullFileFormater::block_file_format(const int file_fd, const int64_t file_size)
{
  if (file_fd < 0)
  {
    return file_fd;
  }

  if (file_size <= 0)
  {
    return file_size;
  }

  int offset = 0;
  int ret = posix_fallocate(file_fd, offset, file_size);
  if (ret != 0)
  {
    return ret;
  }

  return SUCCESS;
}

class Ext3SimpleFileFormater: public FileFormater
{
 public:
  Ext3SimpleFileFormater()
  {
  }
  ~Ext3SimpleFileFormater()
  {
  }

  int block_file_format(const int file_fd, const int64_t file_size);

};

int Ext3SimpleFileFormater::block_file_format(const int file_fd, const int64_t file_size)
{
  if (file_fd < 0)
  {
    return file_fd;
  }

  if (file_size <= 0)
  {
    return file_size;
  }

  int ret = ftruncate(file_fd, file_size);
  if (ret < 0)
  {
    return ret;
  }

  return SUCCESS;
}

} //namespace dataserver
} //namespace dfs
} //namespace neptune

#endif //N_DFS_DS_BLOCKFILEFORMATER_H
