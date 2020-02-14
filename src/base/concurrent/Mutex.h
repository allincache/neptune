#ifndef NEP_BASE_CONCURRENT_MUTEX_H
#define NEP_BASE_CONCURRENT_MUTEX_H 

#include <pthread.h>
#include "Lock.h"
#include "base/include/Base.h"
#include "base/thread/ThreadException.h"

namespace neptune {
namespace base {

class Mutex : noncopyable 
{
 public:
  typedef LockT<Mutex> Lock;
  typedef TryLockT<Mutex> TryLock;

  Mutex();
  ~Mutex();

  void lock() const;
  bool tryLock() const;
  void unlock() const;
  bool willUnlock() const;

 private:
  struct LockState
  {
    pthread_mutex_t* mutex;
  };

  void unlock(LockState&) const;
  void lock(LockState&) const;
  mutable pthread_mutex_t _mutex;
  friend class Cond;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_CONCURRENT_MUTEX_H
