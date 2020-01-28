#ifndef N_DFS_DS_MMAPFILE_OP_H_
#define N_DFS_DS_MMAPFILE_OP_H_

#include "base/fs/FileOp.h"
#include "dfs/util/mmap_file.h"
#include "base/common/Memory.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;

class ParaInfo
{
 public:
  explicit ParaInfo(const int32_t size) :
    flag_(false)
  {
    new_buf_ = new char[size];
    self_buf_ = NULL;
  }

  ~ParaInfo()
  {
    gDeleteA(new_buf_);
  }

  inline char* get_actual_buf() const
  {
    if (flag_)
    {
      return self_buf_;
    }
    else
    {
      return new_buf_;
    }
  }

  inline char* get_new_buf()
  {
    flag_ = false;
    return new_buf_;
  }

  inline bool get_flag() const
  {
    return flag_;
  }

  inline void set_self_buf(char* buf)
  {
    self_buf_ = buf;
    flag_ = true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ParaInfo);
  bool flag_;
  char* self_buf_;
  char* new_buf_;
};

class MMapFileOperation: public FileOperation
{
 public:
  explicit MMapFileOperation(const std::string& file_name, int open_flags = O_RDWR | O_LARGEFILE) :
    FileOperation(file_name, open_flags), is_mapped_(false), map_file_(NULL)
  {

  }

  ~MMapFileOperation()
  {
    gDelete(map_file_);
  }

  int pread_file(char* buf, const int32_t size, const int64_t offset);
  int pread_file(ParaInfo& m_meta_info, const int32_t size, const int64_t offset);
  int pwrite_file(const char* buf, const int32_t size, const int64_t offset);

  int mmap_file(const MMapOption& mmap_option);
  int munmap_file();
  int rename_file();
  void* get_map_data() const;
  int flush_file();

 private:
  MMapFileOperation();
  DISALLOW_COPY_AND_ASSIGN(MMapFileOperation);

  bool is_mapped_;
  MMapFile* map_file_;
};

}
}
}

#endif //N_DFS_DS_MMAPFILE_OP_H_
