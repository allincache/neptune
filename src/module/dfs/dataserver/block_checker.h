#ifndef N_DFS_DS_BLOCKCHECKER_H
#define N_DFS_DS_BLOCKCHECKER_H

#include "base/concurrent/Mutex.h"
#include "logic_block.h"
#include "dataserver_define.h"
#include "file_repair.h"
#include "requester.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class BlockChecker
{
 public:
  BlockChecker() :
    stop_(0), dataserver_id_(0), ds_requester_(NULL)
  {
  }
  ~BlockChecker();

  int init(const uint64_t dataserver_id, Requester* ds_requester);
  void stop();
  int add_repair_task(CrcCheckFile* repair_task);
  int consume_repair_task();
  int do_repair_crc(const CrcCheckFile& check_file);
  int do_repair_eio(const uint32_t blockid);
  int expire_error_block();

 private:
  int check_block(const LogicBlock* logic_block);
  int set_error_bitmap(LogicBlock* logic_block);

  DISALLOW_COPY_AND_ASSIGN(BlockChecker);
  static const int32_t MAX_CHECK_BLOCK_SIZE = 50;

  int stop_;
  uint64_t dataserver_id_;
  FileRepair file_repair_;
  std::deque<CrcCheckFile*> check_file_queue_;
  Mutex check_mutex_;
  Requester* ds_requester_;
};

} //namespace dataserver
} //namespace dfs
} //namespace neptune

#endif //N_DFS_DS_BLOCKCHECKER_H
