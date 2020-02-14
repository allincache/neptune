#ifndef N_DFS_DS_CHECKBLOCK_H
#define N_DFS_DS_CHECKBLOCK_H

#include "base/common/Internal.h"
#include "dfs/util/new_client.h"
#include "dataserver_define.h"
#include "blockfile_manager.h"
#include "logic_block.h"
#include "visit_stat.h"
#include "base/concurrent/Mutex.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class CheckBlock
{

 public:

  CheckBlock()
  {

  }

  ~CheckBlock()
  {
  }

  void add_check_task(const uint32_t block_id);

  void remove_check_task(const uint32_t block_id);

  int check_all_blocks(CheckBlockInfoVec& check_result,
    const int32_t check_flag, const uint32_t check_time, const uint32_t last_check_time);

  int check_one_block(const uint32_t block_id, CheckBlockInfo& result);

  int repair_block_info(const uint32_t block_id);

 private:
  DISALLOW_COPY_AND_ASSIGN(CheckBlock);
  ChangedBlockMap changed_block_map_;
  Mutex changed_block_mutex_;
  uint32_t block_stable_time_;   // minutes
};

} //namespace dataserver
} //namespace dfs
} //namespace neptune

#endif  //N_DFS_DS_CHECKBLOCK_H
