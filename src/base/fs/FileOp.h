#ifndef NEP_BASE_FS_FILEOP_H
#define NEP_BASE_FS_FILEOP_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include "base/common/Internal.h"

namespace neptune {
namespace base {

class FileOperation
{
 public:
  FileOperation(const std::string& file_name, const int open_flags = O_RDWR | O_LARGEFILE);
  virtual ~FileOperation();

  int open_file();
  void close_file();
  virtual int flush_file();

  int flush_data();
  int unlink_file();

  inline char* get_file_name() const
  {
    return file_name_;
  }

  virtual int pread_file(char* buf, const int32_t nbytes, const int64_t offset);
  virtual int pwrite_file(const char* buf, const int32_t nbytes, const int64_t offset);

  int write_file(const char* buf, const int32_t nbytes);

  int64_t get_file_size();
  int ftruncate_file(const int64_t length);
  int seek_file(const int64_t offset);
  int32_t current_pos();

  int get_fd() const
  {
    return fd_;
  }

 protected:
  FileOperation();
  DISALLOW_COPY_AND_ASSIGN(FileOperation);

  int check_file();

 protected:
  static const int MAX_DISK_TIMES = 5;
  static const mode_t OPEN_MODE = 0644;

 protected:
  int fd_;                // file handle
  int open_flags_;        // open flags
  char* file_name_;       // file path name
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_FS_FILEOP_H
