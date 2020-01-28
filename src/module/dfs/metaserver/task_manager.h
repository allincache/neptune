#ifndef N_DFS_MS_TASK_MANAGER_H_
#define N_DFS_MS_TASK_MANAGER_H_

#include "base/common/Internal.h"
#include "base/common/ErrorMsg.h"
#include "base/container/ArrayHelper.h"
#include "task.h"

#ifdef NEP_GTEST
#include <gtest/gtest.h>
#endif

namespace neptune {
namespace dfs {
namespace metaserver {

class TaskManager
{
  #ifdef NEP_GTEST
  friend class TaskManagerTest;
  friend class LayoutManagerTest;
  FRIEND_TEST(TaskManagerTest, add);
  FRIEND_TEST(TaskManagerTest, remove_get_exist);
  FRIEND_TEST(TaskManagerTest, has_space_do_task_in_machine);
  FRIEND_TEST(LayoutManagerTest, update_relation);
  FRIEND_TEST(LayoutManagerTest, build_emergency_replicate_);
  FRIEND_TEST(LayoutManagerTest, check_emergency_replicate_);
  FRIEND_TEST(LayoutManagerTest, build_replicate_task_);
  FRIEND_TEST(LayoutManagerTest, build_compact_task_);
  FRIEND_TEST(LayoutManagerTest, build_balance_task_);
  FRIEND_TEST(LayoutManagerTest, build_redundant_);
  #endif
  typedef std::map<uint64_t, Task*> SERVER_TO_TASK;
  typedef SERVER_TO_TASK::iterator SERVER_TO_TASK_ITER;
  typedef SERVER_TO_TASK::const_iterator SERVER_TO_TASK_CONST_ITER;
  typedef std::map<uint32_t, Task*> BLOCK_TO_TASK;
  typedef BLOCK_TO_TASK::iterator BLOCK_TO_TASK_ITER;
  typedef BLOCK_TO_TASK::const_iterator BLOCK_TO_TASK_CONST_ITER;
  typedef std::set<Task*, TaskCompare> PENDING_TASK;
  typedef PENDING_TASK::iterator PENDING_TASK_ITER;
  typedef PENDING_TASK::const_iterator PENDING_TASK_CONST_ITER;
  struct Machine
  {
    int add(const uint64_t server, Task* task, const bool target = false);
    int remove(const uint64_t server, const bool target = false);
    Task* get(const uint64_t server) const;
    int64_t size(int32_t& source_size, int32_t& target_size) const;
    int64_t size() const;
    SERVER_TO_TASK source_server_to_task_;
    SERVER_TO_TASK target_server_to_task_;
  };
  typedef std::map<uint64_t, Machine> MACHINE_TO_TASK;
  typedef MACHINE_TO_TASK::iterator MACHINE_TO_TASK_ITER;
  typedef MACHINE_TO_TASK::const_iterator MACHINE_TO_TASK_CONST_ITER;
  typedef std::map<uint64_t, Task*> TASKS;
  typedef TASKS::iterator TASKS_ITER;
  typedef TASKS::const_iterator TASKS_CONST_ITER;
  typedef std::list<Task*> GC_QUEUE;
  typedef GC_QUEUE::iterator GC_QUEUE_ITER;
  
 public:
  explicit TaskManager(LayoutManager& manager);
  virtual ~TaskManager();
  int add(const uint32_t id, const std::vector<ServerCollect*>& runer,
        const PlanType type, const time_t now, const PlanPriority priority = PLAN_PRIORITY_NORMAL);
  bool remove(const ArrayHelper<Task*>& tasks);
  int remove(Task* task);
  void clear();

  void dump(DataBuffer& stream) const;
  void dump(const int32_t level) const;

  int64_t get_running_server_size() const;

  Task* get_by_id(const uint64_t id) const;

  bool exist(const uint32_t block) const;
  bool exist(const uint64_t server, bool compact = false) const;
  bool exist(const ArrayHelper<ServerCollect*>& servers, bool compact = false) const;
  bool get_exist_servers(ArrayHelper<ServerCollect*>& result, const ArrayHelper<ServerCollect*>& servers) const;
  bool has_space_do_task_in_machine(const uint64_t server, const bool target) const;
  bool has_space_do_task_in_machine(const uint64_t server) const;

  int remove_block_from_dataserver(const uint64_t server, const uint32_t block, const int64_t seqno, const time_t now);

  int run();
  bool handle(BasePacket* msg, const int64_t seqno);
  int timeout(const time_t now);

  LayoutManager& get_manager() { return manager_;}

 private:
  int remove_(const uint64_t server, const bool target = false);
  int remove_(Task* task);
  bool remove_(const ArrayHelper<Task*>& tasks);
  Task* get_(const uint32_t block) const;
  Task* get_(const BlockCollect* block) const;
  Task* get_(const uint64_t server, bool compact = false) const;
  Task* get_(const ServerCollect* server, bool compact = false) const;
  int64_t get_task_size_in_machine_(int32_t& source_size, int32_t& target_size, const uint64_t server) const;
  int64_t size_() const;
  static int32_t get_gc_queue_index_(const PlanType type);
  static bool is_target(const int32_t index);

  Task* generation_(const uint32_t id, const std::vector<ServerCollect*>& runer,
        const PlanType type, const PlanPriority priority = PLAN_PRIORITY_NORMAL);

  int remove_block_from_dataserver_(const uint64_t server, const uint32_t block, const int64_t seqno, const time_t now);
  
 private:
  static const int8_t TASK_TYPE_NUMS = 4;
  LayoutManager& manager_;
  TASKS tasks_;
  SERVER_TO_TASK  server_to_tasks_;
  MACHINE_TO_TASK machine_to_tasks_;
  BLOCK_TO_TASK block_to_tasks_;
  PENDING_TASK pending_queue_;
  mutable RWLock rwmutex_;
  uint64_t seqno_;
};

}
}
}

#endif
