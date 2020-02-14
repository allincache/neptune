#ifndef N_BASE_COMMON_FSNAME_H
#define N_BASE_COMMON_FSNAME_H

#include <string>
#include "base/common/Internal.h"

namespace neptune {
namespace base {

struct FileBits
{
  uint32_t block_id_;
  uint32_t seq_id_;
  uint32_t suffix_;
};

class FSName
{
 public:
  FSName();
  FSName(const uint32_t block_id, const uint64_t file_id, const int32_t cluster_id = 0);
  FSName(const char *file_name, const char* suffix = NULL, const int32_t cluster_id = 0);
  virtual ~FSName();

  const char* get_name(const bool large_flag = false);
  void set_name(const char* file_name, const char* suffix = NULL, const int32_t cluster_id = 0);
  void set_suffix(const char* suffix);
  std::string to_string();

  static BaseFileType check_file_type(const char* dfs_name);

  inline bool is_valid() const
  {
    return is_valid_;
  }

  inline void set_block_id(const uint32_t id)
  {
    file_.block_id_ = id;
  }

  inline uint32_t get_block_id() const
  {
    return file_.block_id_;
  }

  inline void set_seq_id(const uint32_t id)
  {
    file_.seq_id_ = id;
  }

  inline uint32_t get_seq_id() const
  {
    return file_.seq_id_;
  }

  inline void set_suffix(const uint32_t id)
  {
    file_.suffix_ = id;
  }

  inline uint32_t get_suffix() const
  {
    return file_.suffix_;
  }

  inline void set_file_id(const uint64_t id)
  {
    file_.suffix_ = (id >> 32);
    file_.seq_id_ = (id & 0xFFFFFFFF);
  }

  inline uint64_t get_file_id()
  {
    uint64_t id = file_.suffix_;
    return ((id << 32) | file_.seq_id_);
  }

  inline void set_cluster_id(const int32_t cluster_id)
  {
    cluster_id_ = cluster_id;
  }

  inline int32_t get_cluster_id() const
  {
    return cluster_id_;
  }

 private:
  void encode(const char * input, char *output);
  void decode(const char * input, char *output);

 private:
  bool is_valid_;
  FileBits file_;
  int32_t cluster_id_;
  char file_name_[DFS_FILE_LEN];
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_FSNAME_H
