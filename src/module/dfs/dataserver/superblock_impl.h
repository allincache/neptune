#ifndef N_DFS_DS_SUPERBLOCK_IMPL_H_
#define N_DFS_DS_SUPERBLOCK_IMPL_H_

#include <string>
#include "base/fs/FileOp.h"
#include "bit_map.h"
#include "base/common/ErrorMsg.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;

class SuperBlockImpl
{
 public:
  SuperBlockImpl(const std::string& filename, const int32_t super_start_offset, const bool flag = true);
  ~SuperBlockImpl();

  int read_super_blk(SuperBlock& super_block) const;
  int write_super_blk(const SuperBlock& super_block);
  bool check_status(const char* block_tag, const SuperBlock& super_block) const;
  int read_bit_map(char* buf, const int32_t size) const;
  int write_bit_map(const BitMap* normal_bitmap, const BitMap* error_bitmap);
  int flush_file();

  int dump_super_block(const SuperBlock&)
  {
    return SUCCESS;
  }
  int backup_super_block(const SuperBlock&)
  {
    return SUCCESS;
  }

 private:
  SuperBlockImpl();
  DISALLOW_COPY_AND_ASSIGN(SuperBlockImpl);

  std::string super_block_file_; // associate super block file
  int32_t super_reserve_offset_; // super block reserved offset
  int32_t bitmap_start_offset_;  // super block bitmap offset
  FileOperation* file_op_;
};

}
}
}

#endif //N_DFS_DS_SUPERBLOCK_IMPL_H_
