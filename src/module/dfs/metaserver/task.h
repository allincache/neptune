#ifndef N_DFS_MS_TASK_H_
#define N_DFS_MS_TASK_H_

#include <stdint.h>
#include "base/common/Shared.h"
#include "base/common/Handle.h"
#include "base/time/Timer.h"
#include "gc.h"
#include "ns_define.h"
#include "base/concurrent/Lock.h"
#include "base/common/Internal.h"

#ifdef NEP_GTEST
#include <gtest/gtest.h>
#endif

namespace neptune {
namespace dfs {
namespace metaserver {

class TaskManager;
class Task: public GCObject
{
  friend class TaskManager;
  #ifdef NEP_GTEST
  friend class TaskTest;
  friend class TaskManagerTest;
  friend class LayoutManagerTest;
  FRIEND_TEST(TaskTest, dump);
  FRIEND_TEST(TaskManagerTest, add);
  FRIEND_TEST(LayoutManagerTest, build_emergency_replicate_);
  FRIEND_TEST(LayoutManagerTest, check_emergency_replicate_);
  FRIEND_TEST(LayoutManagerTest, build_replicate_task_);
  FRIEND_TEST(LayoutManagerTest, build_compact_task_);
  FRIEND_TEST(LayoutManagerTest, build_balance_task_);
  FRIEND_TEST(LayoutManagerTest, build_redundant_);
  #endif
 
 public:
  Task(TaskManager& manager, const PlanType type,
      const PlanPriority priority, const uint32_t block_id,
      const std::vector<ServerCollect*>& runer);
  virtual ~ Task(){};
  virtual int handle() = 0;
  virtual int handle_complete(BasePacket* msg, bool& all_complete_flag) = 0;
  virtual void dump(DataBuffer& stream);
  virtual void dump(const int32_t level, const char* const format = NULL);
  virtual void runTimerTask();
  bool operator < (const Task& task) const;
  bool need_add_server_to_map() const;
  bool timeout(const time_t now) const;
  
 protected:
  std::vector<ServerCollect*> runer_;
  uint32_t block_id_;
  PlanType type_;
  PlanStatus status_;
  PlanPriority priority_;
  int64_t seqno_;
  TaskManager& manager_;
 
 private:
  DISALLOW_COPY_AND_ASSIGN(Task);
};

struct TaskCompare
{
  bool operator()(const Task* lhs, const Task* rhs) const
{
  return (*lhs) < (*rhs);
}
};

class CompactTask: public Task
{
  #ifdef NEP_GTEST
  friend class TaskTest;
  FRIEND_TEST(TaskTest, compact_handle);
  FRIEND_TEST(TaskTest, compact_task_do_complete);
  FRIEND_TEST(TaskTest, compact_task_check_complete);
  #endif
  struct CompactComplete
  {
    uint64_t id_;
    uint32_t block_id_;
    PlanStatus status_;
    bool all_success_;
    bool has_success_;
    bool is_complete_;
    bool current_complete_result_;
    BlockInfo block_info_;
    CompactComplete(const uint64_t id, const uint32_t block_id, const PlanStatus status):
      id_(id), block_id_(block_id), status_(status), all_success_(true),
      has_success_(false), is_complete_(true), current_complete_result_(false){}
  };
 
 public:
  CompactTask(TaskManager& manager, const PlanPriority priority,
      const uint32_t block_id, const std::vector<ServerCollect*>& runer);
  virtual ~CompactTask(){}
  virtual int handle();
  virtual int handle_complete(BasePacket* msg, bool& all_complete_flag);
  virtual void runTimerTask();
  virtual void dump(DataBuffer& stream);
  virtual void dump(const int32_t level, const char* const format = NULL);
  private:
  void check_complete(CompactComplete& value, VUINT64& ds_list);
  int do_complete(CompactComplete& value, VUINT64& ds_list);
  static CompactStatus status_transform_plan_to_compact(const PlanStatus status);
  static PlanStatus status_transform_compact_to_plan(const CompactStatus status);
  private:
  static const int8_t INVALID_SERVER_ID;
  static const int8_t INVALID_BLOCK_ID;
  std::vector< std::pair <uint64_t, PlanStatus> > complete_status_;
  BlockInfo block_info_;
  Mutex mutex_;
  private:
  DISALLOW_COPY_AND_ASSIGN(CompactTask);
};

class ReplicateTask : public Task
{
  #ifdef NEP_GTEST
  friend class TaskTest;
  FRIEND_TEST(TaskTest, replicate_task_handle_complete);
  #endif
 
 public:
  ReplicateTask(TaskManager& manager, const PlanPriority priority,
    const uint32_t block_id, const std::vector<ServerCollect*>& runer);
  virtual ~ReplicateTask(){}
  virtual int handle();
  virtual int handle_complete(BasePacket* msg, bool& all_complete_flag);
 
 protected:
  ReplicateBlockMoveFlag flag_;
 
 private:
  DISALLOW_COPY_AND_ASSIGN(ReplicateTask);
};

class DeleteTask : public Task
{
 public:
  DeleteTask(TaskManager& manager, const PlanPriority priority,
    const uint32_t block_id, const std::vector<ServerCollect*>& runer);
  virtual ~DeleteTask(){}
  virtual int handle();
  virtual int handle_complete(BasePacket* msg, bool& all_complete_flag);
 
 private:
  DISALLOW_COPY_AND_ASSIGN(DeleteTask);
};

class MoveTask : public ReplicateTask
{
 public:
  MoveTask(TaskManager& manager, const PlanPriority priority,
    const uint32_t block_id, const std::vector<ServerCollect*>& runer);
  virtual ~MoveTask(){}
 
 private:
  DISALLOW_COPY_AND_ASSIGN(MoveTask);
};

}
}
}

#endif

