#ifndef N_DFS_CLIENT_DFSLARGEFILE_H_
#define N_DFS_CLIENT_DFSLARGEFILE_H_

#include <stdint.h>
#include "base/common/Internal.h"
#include "dfs_file.h"
#include "local_key.h"

namespace neptune {
namespace dfs {

class DfsLargeFile : public DfsFile
{
public:
  DfsLargeFile();
  virtual ~DfsLargeFile();

  virtual int open(const char* file_name, const char* suffix, const int flags, ... );
  virtual int64_t read(void* buf, const int64_t count);
  virtual int64_t readv2(void* buf, const int64_t count, DfsFileStat* file_info);
  virtual int64_t write(const void* buf, const int64_t count);
  virtual int64_t lseek(const int64_t offset, const int whence);
  virtual int64_t pread(void* buf, const int64_t count, const int64_t offset);
  virtual int64_t pwrite(const void* buf, const int64_t count, const int64_t offset);
  virtual int fstat(DfsFileStat* file_info, const DfsStatType mode = NORMAL_STAT);
  virtual int close();
  virtual int64_t get_file_length();
  virtual int unlink(const char* file_name, const char* suffix, int64_t& file_size, const DfsUnlinkType action);

protected:
  virtual int64_t get_segment_for_read(const int64_t offset, char* buf, const int64_t count);
  virtual int64_t get_segment_for_write(const int64_t offset, const char* buf, const int64_t count);
  virtual int read_process(int64_t& read_size, const InnerFilePhase read_file_phase = FILE_PHASE_READ_FILE);
  virtual int write_process();
  virtual int32_t finish_write_process(const int status);
  virtual int close_process();
  virtual int unlink_process();
  virtual int wrap_file_info(DfsFileStat* file_stat, FileInfo* file_info);

private:
  int upload_key();
  int load_meta(int32_t flags);
  int load_meta_head();

private:
  LocalKey local_key_;
  char* meta_suffix_;
};


}
}
#endif  // N_DFS_CLIENT_DFSLARGEFILE_H_
