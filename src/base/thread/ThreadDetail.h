#ifndef N_BASE_COMMON_THREAD_H
#define N_BASE_COMMON_THREAD_H

#include "base/common/Shared.h"
#include "base/common/Handle.h"
#include "base/concurrent/Mutex.h"
#include "base/concurrent/Cond.h"
#include "base/time/Time.h"

namespace neptune {
namespace base {

class Thread : virtual public Shared
{

 public:
  Thread();
  virtual ~Thread();
  virtual void run() = 0;
  int  start(size_t stackSize= 0);
  bool isAlive() const; 
  void _done(); 
  int join();  
  int detach();
  pthread_t id() const;
  static void yield();
  static void ssleep(const Time& timeout);

 protected:
  bool  _running;   
  bool _started;    
  bool _detachable;
  pthread_t _thread;
  Mutex _mutex;

 private:
  Thread(const Thread&);            
  Thread& operator=(const Thread&);   
};

typedef Handle<Thread> ThreadPtr;

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_THREAD_H

