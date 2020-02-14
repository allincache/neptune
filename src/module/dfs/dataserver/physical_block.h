#ifndef N_DFS_DS_PHYSICALBLOCK_H_
#define N_DFS_DS_PHYSICALBLOCK_H_

#include <string>
#include "base/fs/FileOp.h"
#include "mmap_file_op.h"
#include "dataserver_define.h"
#include "dfs/util/config_item.h"

namespace neptune {
namespace dfs {
namespace dataserver {

static const int32_t BLOCK_RESERVER_LENGTH = 512;

class PhysicalBlock
{
 public:
  PhysicalBlock(const uint32_t physical_block_id, const std::string& mount_path, const int32_t block_length,
      const BlockType block_type);
  ~PhysicalBlock();

  int clear_block_prefix();
  void set_block_prefix(const uint32_t block_id, const uint32_t prev_physic_blockid,
      const uint32_t next_physical_blockid);

  static int init_prefix_op(std::string& mount_path);  // mmap block_prefix file

  int load_block_prefix();
  int dump_block_prefix();
  void get_block_prefix(BlockPrefix& block_prefix);

  int pread_data(char* buf, const int32_t nbytes, const int32_t offset);
  int pwrite_data(const char* buf, const int32_t nbytes, const int32_t offset);

  inline int32_t get_total_data_len() const
  {
    return total_data_len_;
  }

  inline uint32_t get_physic_block_id() const
  {
    return physical_block_id_;
  }

  inline void set_next_block(const uint32_t next_physical_block_id)
  {
    block_prefix_.next_physic_blockid_ = next_physical_block_id;
    return;
  }

  inline BlockPrefix* get_block_prefix()
  {
    return &block_prefix_;
  }

  inline int get_block_fd()
  {
    return file_op_->get_fd();
  }

  static void destroy_prefix_op()
  {
    if (NULL != prefix_op_)
    {
      prefix_op_->munmap_file();
      gDelete(prefix_op_);
    }
  }

 private:
  PhysicalBlock();
  DISALLOW_COPY_AND_ASSIGN(PhysicalBlock);

  uint32_t physical_block_id_; // physical block id

  int32_t data_start_; // the data start offset of this block file
  int32_t total_data_len_; // total data size
  BlockPrefix block_prefix_; // block meta info prefix
  FileOperation* file_op_;   // file operation handle
  static MMapFileOperation* prefix_op_;     // block prefix op
};

}
}
}

#endif //N_DFS_DS_PHYSICALBLOCK_H_
