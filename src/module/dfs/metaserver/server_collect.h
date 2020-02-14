#ifndef N_DFS_MS_SERVER_COLLECT_H_
#define N_DFS_MS_SERVER_COLLECT_H_

#include <stdint.h>
#include "base/network/simple/net.h"
#include "ns_define.h"
#include "base/concurrent/Lock.h"
#include "dfs/util/parameter.h"
#include "base/container/Vector.h"
#include "global_factory.h"
#include "block_manager.h"

#ifdef NEP_GTEST
#include <gtest/gtest.h>
#endif

namespace neptune {
namespace dfs {
namespace metaserver {

class BlockCollect;
class LayoutManager;
class ServerCollect : public GCObject
{
  friend class BlockCollect;
  friend class LayoutManager;
  typedef BaseSortedVector<BlockCollect*, BlockIdCompare>::iterator BLOCKS_ITER;
  #ifdef NEP_GTEST
  friend class ServerCollectTest;
  friend class LayoutManagerTest;
  FRIEND_TEST(ServerCollectTest, add);
  FRIEND_TEST(ServerCollectTest, remove);
  FRIEND_TEST(ServerCollectTest, get_range_blocks_);
  FRIEND_TEST(ServerCollectTest, touch);
  FRIEND_TEST(LayoutManagerTest, update_relation);
 
 public:
  bool exist_writable(const BlockCollect* block);
  #endif
 
 public:
  explicit ServerCollect(const uint64_t id);
  ServerCollect(const DataServerStatInfo& info, const time_t now);
  virtual ~ServerCollect();

  bool add(const BlockCollect* block, const bool master, const bool writable);
  bool remove(BlockCollect* block);
  bool remove(const ArrayHelper<BlockCollect*>& blocks);
  void update(const DataServerStatInfo& info, const time_t now, const bool is_new);
  void statistics(NsGlobalStatisticsInfo& stat, const bool is_new) const;
  bool add_writable(const BlockCollect* block);
  bool clear(LayoutManager& manager, const time_t now);
  bool touch(bool& promote, int32_t& count, const double average_used_capacity);
  bool get_range_blocks(ArrayHelper<BlockCollect*>& blocks, const uint32_t begin, const int32_t count) const;
  int scan(SSMScanParameter& param, const int8_t scan_flag) const;
  void callback(LayoutManager& manager);
  void reset(LayoutManager& manager, const DataServerStatInfo& info, const time_t now);

  inline int64_t use_capacity() const { return use_capacity_;}
  inline int64_t total_capacity() const { return total_capacity_;}
  inline uint64_t id() const { return id_;}
  inline bool is_full() const { return use_capacity_ >= total_capacity_ * SYSPARAM_NAMESERVER.max_use_capacity_ratio_ / 100;}
  inline bool is_alive(const time_t now) const { return ((now < last_update_time_+ SYSPARAM_NAMESERVER.heart_interval_ * MULTIPLE));}
  inline bool is_alive() const { return (status_ == DATASERVER_STATUS_ALIVE);}
  inline void update_status() { status_ = DATASERVER_STATUS_DEAD;}
  inline int32_t block_count() const { return block_count_;}

  inline bool is_equal_group(const uint32_t id) const
  {
    return (static_cast<int32_t>(id % SYSPARAM_NAMESERVER.group_count_)
            == SYSPARAM_NAMESERVER.group_seq_);
  }
  inline void set_report_block_status(const int8_t status)
  {
    rb_status_ = status;
  }
  inline bool is_report_block(bool& rb_expire, const time_t now, const bool isnew) const
  {
    if (!isnew)
      rb_expire = is_report_block_expired(now);
    //LOG(DEBUG, "%s, rb_expire: %d,rb_expired: %ld, status: %d, next: %ld, now: %ld, isnew: %d",
    //    CNetUtil::addrToString(id()).c_str(),
    //    rb_expire, rb_expired_time_, rb_status_, next_report_block_time_, now, isnew);
    return (isnew || rb_expire) ? true : now >= next_report_block_time_;
  }
  inline bool is_report_block_expired(const time_t now) const
  {
    return (now > rb_expired_time_ && rb_status_ == REPORT_BLOCK_STATUS_REPORTING);
  }
  inline bool is_report_block_complete(void) const
  {
    return rb_status_ == REPORT_BLOCK_STATUS_COMPLETE;
  }
  inline void set_report_block_info(const time_t now, const int8_t status)
  {
    rb_expired_time_ = now + SYSPARAM_NAMESERVER.report_block_expired_time_;
    rb_status_ = status;
  }
  inline bool is_in_dead_queue_timeout(const time_t now) const
  {
    return now >= in_dead_queue_timeout_;
  }
  inline void set_in_dead_queue_timeout(const time_t now)
  {
    in_dead_queue_timeout_ = now + SYSPARAM_NAMESERVER.replicate_wait_time_;
  }

  void set_next_report_block_time(const time_t now, const int64_t time_seed, const bool ns_switch);
  int choose_writable_block(BlockCollect*& result);
  int choose_writable_block_force(BlockCollect*& result) const;
  int choose_move_block_random(BlockCollect*& result) const;
  int expand_ratio(const float expand_ratio = 0.1);

  static const int8_t MULTIPLE;
  static const int8_t AVERAGE_USED_CAPACITY_MULTIPLE;
  
 private:
  DISALLOW_COPY_AND_ASSIGN(ServerCollect);
  bool clear_();
  bool remove_(BlockCollect* block);
  int choose_writable_block_(BlockCollect*& result) const;
  bool remove_writable_(const ArrayHelper<BlockCollect*>& blocks);
  bool get_range_blocks_(ArrayHelper<BlockCollect*>& blocks, const uint32_t begin, const int32_t count) const;

 private:
  uint64_t id_;
  int64_t write_byte_;
  int64_t read_byte_;
  int64_t write_count_;
  int64_t read_count_;
  int64_t unlink_count_;
  int64_t use_capacity_;
  int64_t total_capacity_;
  time_t  startup_time_;
  time_t  rb_expired_time_;
  time_t  next_report_block_time_;
  time_t  in_dead_queue_timeout_;
  int32_t current_load_;
  int32_t block_count_;
  mutable int32_t write_index_;
  int32_t writable_index_;
  int16_t  status_;
  int16_t  rb_status_;//report block complete status
  BaseSortedVector<BlockCollect*, BlockIdCompare>* hold_;
  BaseSortedVector<BlockCollect*, BlockIdCompare>* writable_;
  VectorHelper<BlockCollect*>* hold_master_;
  mutable RWLock mutex_;
};

}
}
}

#endif
