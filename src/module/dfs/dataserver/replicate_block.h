#ifndef N_DFS_DS_REPLICATEBLOCK_H_
#define N_DFS_DS_REPLICATEBLOCK_H_

#include "dfs/util/new_client.h"
#include "dataserver_define.h"
#include "blockfile_manager.h"
#include "logic_block.h"
#include "base/concurrent/Mutex.h"
#include "base/common/Monitor.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class ReplicateBlock
{
 public:
  explicit ReplicateBlock(const uint64_t ns_ip);
  ~ReplicateBlock();

  void stop();
  int add_repl_task(ReplBlockExt& repl_blk);

  int add_cloned_block_map(const uint32_t block_id);
  int del_cloned_block_map(const uint32_t block_id);
  int expire_cloned_block_map();
  int run_replicate_block();

 private:
  void init();
  int replicate_block_to_server(const ReplBlockExt& b);
  int send_repl_block_complete_info(const int status, const ReplBlockExt& b);
  int clear_cloned_block_map();

  ReplicateBlock();
  DISALLOW_COPY_AND_ASSIGN(ReplicateBlock);

  int stop_;
  std::deque<ReplBlockExt> repl_block_queue_; // repl block queue
  ReplBlockMap replicating_block_map_; // replicating
  Monitor<Mutex> repl_block_monitor_;

  ClonedBlockMap cloned_block_map_;
  Mutex cloned_block_mutex_;
  int32_t expire_cloned_interval_; // check interval
  int32_t last_expire_cloned_block_time_;

  uint64_t ns_ip_;
};

}
}
}
#endif  //N_DFS_DS_REPLICATEBLOCK_H_
