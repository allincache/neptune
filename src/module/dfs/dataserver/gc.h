#ifndef N_DFS_DS_GC_H
#define N_DFS_DS_GC_H

#include "base/time/Timer.h"
#include "base/concurrent/Mutex.h"
#include "dataserver_define.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class GCObjectManager
{
public:
  GCObjectManager();
  virtual ~GCObjectManager();
  int add(GCObject* object);
  void run();
  int initialize(TimerPtr timer);
  int wait_for_shut_down();
  void destroy();
  bool is_init() { return is_init_; }
  static GCObjectManager& instance()
  {
    return instance_;
  }
#if defined(NEP_GTEST) || defined(DFS_DS_INTEGRATION)
public:
#else
private:
#endif
  DISALLOW_COPY_AND_ASSIGN(GCObjectManager);
  std::list<GCObject*> object_list_;
  Mutex mutex_;
  static GCObjectManager instance_;
  bool is_init_;
  bool destroy_;

#if defined(NEP_GTEST) || defined(DFS_DS_INTEGRATION)
public:
#else
private:
#endif
  class ExpireTimerTask: public TimerTask
  {
  public:
    explicit ExpireTimerTask(GCObjectManager& manager)
      :manager_(manager)
    {

    }

    virtual ~ExpireTimerTask() {}
    virtual void runTimerTask();
  private:
    GCObjectManager& manager_;
  };
  typedef Handle<ExpireTimerTask> ExpireTimerTaskPtr;
};

}
}
}

#endif //N_DFS_DS_GC_H
