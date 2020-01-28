#ifndef N_DFS_MS_OPERATION_LOG_SYNC_MANAGER_H_
#define N_DFS_MS_OPERATION_LOG_SYNC_MANAGER_H_

#include <deque>
#include <map>
#include "base/concurrent/Mutex.h"
#include "base/common/Monitor.h"
#include "base/time/Timer.h"
#include "dfs/util/file_queue.h"
#include "dfs/util/file_queue_thread.h"
#include "dfs/message/message_factory.h"
#include "oplog.h"
#include "block_id_factory.h"

namespace neptune {
namespace dfs {
namespace metaserver {

class LayoutManager;
class OpLogSyncManager: public IPacketQueueHandler
{
  friend class FlushOpLogTimerTask;
 
 public:
  explicit OpLogSyncManager(LayoutManager& mm);
  virtual ~OpLogSyncManager();
  int initialize();
  int wait_for_shut_down();
  int destroy();
  int register_slots(const char* const data, const int64_t length) const;
  void switch_role();
  int log(const uint8_t type, const char* const data, const int64_t length, const time_t now);
  int push(BasePacket* msg, int32_t max_queue_size = 0, bool block = false);
  inline FileQueueThread* get_file_queue_thread() const { return file_queue_thread_;}
  int replay_helper(const char* const data, const int64_t data_len, int64_t& pos, const time_t now = Func::get_monotonic_time());
  int replay_helper_do_msg(const int32_t type, const char* const data, const int64_t data_len, int64_t& pos);
  int replay_helper_do_oplog(const time_t now, const int32_t type, const char* const data, const int64_t data_len, int64_t& pos);

  inline uint32_t generation(const uint32_t id = 0) { return id_factory_.generation(id);}
 
 private:
  DISALLOW_COPY_AND_ASSIGN( OpLogSyncManager);
  virtual bool handlePacketQueue(Packet *packet, void *args);
  static int sync_log_func(const void* const data, const int64_t len, const int32_t threadIndex, void *arg);
  int send_log_(const char* const data, const int64_t length);
  int transfer_log_msg_(BasePacket* msg);
  int recv_log_(BasePacket* msg);
  int replay_all_();
 
 private:
  LayoutManager& manager_;
  OpLog* oplog_;
  FileQueue* file_queue_;
  FileQueueThread* file_queue_thread_;
  BlockIdFactory id_factory_;
  Mutex mutex_;
  PacketQueueThread work_thread_;
};

} //namespace metaserver
}
} //namespace neptune

#endif
