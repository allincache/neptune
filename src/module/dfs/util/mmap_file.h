#ifndef N_DFS_UTIL_MMAPFILE_H_
#define N_DFS_UTIL_MMAPFILE_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "base/common/Internal.h"

namespace neptune {
namespace dfs {

using namespace neptune::base;

class MMapFile
{
 public:
  MMapFile();
  explicit MMapFile(const int fd);
  MMapFile(const MMapOption& mmap_option, const int fd);
  ~MMapFile();

  bool sync_file();
  bool map_file(const bool write = false);
  bool remap_file();
  void* get_data() const;
  int32_t get_size() const;
  bool munmap_file();

 private:
  bool ensure_file_size(const int32_t size);
  int32_t size_;
  int fd_;
  void* data_;
  MMapOption mmap_file_option_;
};

} //namespace dfs
} //namespace neptune

#endif
