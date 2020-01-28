#ifndef N_DFS_DS_BLOCKFILEMANAGER_H
#define N_DFS_DS_BLOCKFILEMANAGER_H

#include <string>
#include <list>
#include <vector>
#include <set>
#include <dirent.h>
#include "physical_block.h"
#include "logic_block.h"
#include "bit_map.h"
#include "superblock_impl.h"
#include "dfs/util/parameter.h"
#include "base/concurrent/Lock.h"


namespace neptune {
namespace dfs {
namespace dataserver {

class LogicBlock;
class BlockFileManager
{
 
 public:
  static BlockFileManager* get_instance()
  {
    static BlockFileManager s_blockfile_manager;
    return &s_blockfile_manager;
  }

 public:
  int format_block_file_system(const FileSystemParameter& fs_param, const bool speedup = false);
  int clear_block_file_system(const FileSystemParameter& fs_param);
  int bootstrap(const FileSystemParameter& fs_param);

  int new_block(const uint32_t logic_block_id, uint32_t& physical_block_id, const BlockType block_type =
      C_MAIN_BLOCK);
  int new_ext_block(const uint32_t logic_block_id, const uint32_t physical_block_id,
      uint32_t& ext_physical_block_id, PhysicalBlock **tPhysicalBlock);
  int del_block(const uint32_t logic_block_id, const BlockType block_type = C_MAIN_BLOCK);

  LogicBlock* get_logic_block(const uint32_t logic_block_id, const BlockType block_type = C_MAIN_BLOCK);
  int get_all_logic_block(std::list<LogicBlock*>& logic_block_list, const BlockType block_type = C_MAIN_BLOCK);
  int get_all_block_info(std::set<BlockInfo>& blocks, const BlockType block_type = C_MAIN_BLOCK);
  int64_t get_all_logic_block_size(const BlockType block_type = C_MAIN_BLOCK);
  int get_logic_block_ids(VUINT& logic_block_ids, const BlockType block_type = C_MAIN_BLOCK);
  int get_all_physic_block(std::list<PhysicalBlock*>& physic_block_list);

  //status info
  int query_super_block(SuperBlock& super_block_info);
  int query_approx_block_count(int32_t& block_count) const;
  int query_bit_map(char** bit_map_buffer, int32_t& bit_map_len, int32_t& set_count, const BitMapType bitmap_type =
      C_ALLOCATE_BLOCK);
  int query_space(int64_t& used_bytes, int64_t& total_bytes) const;

  int load_super_blk(const FileSystemParameter& fs_param);

  int switch_compact_blk(const uint32_t block_id);
  int expire_compact_blk(const time_t time, std::set<uint32_t>& erase_blocks);

  int set_error_bitmap(const std::set<uint32_t>& error_blocks);
  int reset_error_bitmap(const std::set<uint32_t>& reset_error_blocks);

  int create_block_prefix();
  void clear_block_tmp_index(const char* mount_name);

 private:
  BlockFileManager()
  {
  }
  ~BlockFileManager();
  DISALLOW_COPY_AND_ASSIGN(BlockFileManager);

  int load_block_file();

  int init_super_blk_param(const FileSystemParameter& fs_param);
  void calc_block_count(const int64_t avail_data_space, int32_t& main_block_count, int32_t& ext_block_count);
  int create_fs_super_blk();
  int create_fs_dir();
  uint32_t calc_bitmap_count();
  int create_block(BlockType block_type);

  int find_avail_block(uint32_t& ext_physical_block_id, const BlockType block_type);

  LogicBlock* choose_del_block(const uint32_t logic_block_id, BlockType& block_type);
  int erase_logic_block(const uint32_t logic_block_id, const BlockType block_type);
  void destruct_logic_blocks(const BlockType block_type);
  void destruct_physic_blocks();

  void rollback_superblock(const uint32_t physical_block_id, const bool modify_flag,
      BlockType type = C_MAIN_BLOCK);

  static int index_filter(const struct dirent *entry);

 private:
  static const int32_t INDEXFILE_SAFE_MULT = 4;
  static const int32_t INNERFILE_MAX_MULTIPE = 30;

#if defined(DFS_DS_GTEST)
 public:
#else
#endif
  typedef std::map<uint32_t, LogicBlock*> LogicBlockMap;
  typedef LogicBlockMap::iterator LogicBlockMapIter;
  typedef std::map<uint32_t, PhysicalBlock*> PhysicalBlockMap;
  typedef PhysicalBlockMap::iterator PhysicalBlockMapIter;

  LogicBlockMap logic_blocks_; // logic blocks
  LogicBlockMap compact_logic_blocks_; // compact logic blocks
  PhysicalBlockMap physical_blocks_;   // physical blocks

  int bit_map_size_;      // bitmap size
  BitMap* normal_bit_map_; // normal bitmap
  BitMap* error_bit_map_;  // error bitmap
  SuperBlock super_block_; // super block
  SuperBlockImpl* super_block_impl_; // super block implementation handle
  RWLock rw_lock_;           // read-write lock
};

} //namespace dataserver
} //namespace dfs
} //namespace neptune

#endif //N_DFS_DS_BLOCKFILEMANAGER_H
