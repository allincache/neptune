#ifndef NEP_BASE_THREADPOOL_TP_H 
#define NEP_BASE_THREADPOOL_TP_H

#include <list>
#include <deque>
#include <vector>
#include "base/common/Shared.h"
#include "base/concurrent/Mutex.h"
#include "base/concurrent/CountMutex.h"
#include "base/concurrent/Cond.h"
#include "base/common/Monitor.h"
#include "base/thread/Thread.h"
#include "base/common/EventHandler.h"

#define DEFAUTL_STACK_SIZE 4*1024*1024   //16M
#define DEFALUT_LIST_SIZE_MAX 40960 

namespace neptune {
namespace base {

class ThreadPool : public Monitor<Mutex>
{

 public:
  ThreadPool(int size = 1 , int sizeMax=1,int sizeWarn=1,
              int listSizeMax= DEFALUT_LIST_SIZE_MAX,
              int stackSize=DEFAUTL_STACK_SIZE);
  virtual ~ThreadPool();

  void destroy();
  int  execute(ThreadPoolWorkItem* workItem);
  inline void promoteFollower(pthread_t thid);
  void joinWithAllThreads();
  bool isMaxCapacity() const;

private:

  bool run(pthread_t thid); // Returns true if a follower should be promoted.

  bool _destroyed;

  std::list<ThreadPoolWorkItem*> _workItems;
  int _listSize;
  int _procSize;
  const int _listSizeMax;
  Monitor<Mutex> _monitor; 
  class EventHandlerThread : public Thread
  {
   public:
    EventHandlerThread(const ThreadPool*);
    virtual void run();

   private:
    ThreadPool* _pool;
  };

  friend class EventHandlerThread;

  const int _size; // Number of threads that are pre-created.
  const int _sizeMax; // Maximum number of threads.
  const int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
  const size_t _stackSize;

  std::vector<ThreadPtr> _threads; // All threads, running or not.
  int _running; // Number of running threads.
  int _inUse; // Number of threads that are currently in use.
  double _load; // Current load in number of threads.

  bool _promote;
  volatile int _waitingNumber;
};

} //namespace base
} //namespace neptune
#endif //NEP_BASE_THREADPOOL_TP_H
