#ifndef NEP_BASE_CONCURRENT_CountMutex_H
#define NEP_BASE_CONCURRENT_CountMutex_H

#include "Lock.h"
#include "base/thread/ThreadException.h"

namespace neptune {
namespace base {

class Cond;

class CountMutex : noncopyable
{

 public:
  typedef LockT<CountMutex> Lock;
  typedef TryLockT<CountMutex> TryLock;

  CountMutex();
  ~CountMutex();

  void lock() const;
  bool tryLock() const;
  void unlock() const;
  bool willUnlock() const;

 private:
  struct LockState
  {
    pthread_mutex_t* mutex;
    int count;
  };

  void unlock(LockState&) const;
  void lock(LockState&) const;

  friend class Cond;
  mutable pthread_mutex_t _mutex;
  mutable int _count;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_CONCURRENT_CountMutex_H
