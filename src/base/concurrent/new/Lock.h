#ifndef BASE_SYNCHRONIZATION_LOCK_H_
#define BASE_SYNCHRONIZATION_LOCK_H_


#include "base/concurrent/new/LockImpl.h"
#include "base/thread/new/PosixThread.h"


namespace base {

class Lock {
 public:
  Lock();
  ~Lock();

  void Acquire() {
    lock_.Lock();
    CheckUnheldAndMark();
  }

  void Release() {
    CheckHeldAndUnmark();
    lock_.Unlock();
  }

  bool Try() {
    bool rv = lock_.Try();
    if (rv) {
      CheckUnheldAndMark();
    }
    return rv;
  }

  void AssertAcquired() const;

  static bool HandlesMultipleThreadPriorities() {
    return internal::LockImpl::PriorityInheritanceAvailable();
  }

  friend class ConditionVariable;

 private:
  void CheckHeldAndUnmark();
  void CheckUnheldAndMark();
  base::PosixThreadRef owning_thread_ref_;
  internal::LockImpl lock_;

  //DISALLOW_COPY_AND_ASSIGN(Lock);
};

// A helper class that acquires the given Lock while the AutoLock is in scope.
class AutoLock {
 public:
  struct AlreadyAcquired {};

  explicit AutoLock(Lock& lock) : lock_(lock) {
    lock_.Acquire();
  }

  AutoLock(Lock& lock, const AlreadyAcquired&)
      : lock_(lock) {
    lock_.AssertAcquired();
  }

  ~AutoLock() {
    lock_.AssertAcquired();
    lock_.Release();
  }

 private:
  Lock& lock_;
  //DISALLOW_COPY_AND_ASSIGN(AutoLock);
};

// AutoUnlock is a helper that will Release() the |lock| argument in the
// constructor, and re-Acquire() it in the destructor.
class AutoUnlock {
 public:
  explicit AutoUnlock(Lock& lock) : lock_(lock) {
    // We require our caller to have the lock.
    lock_.AssertAcquired();
    lock_.Release();
  }

  ~AutoUnlock() {
    lock_.Acquire();
  }

 private:
  Lock& lock_;
  //DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
};

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_LOCK_H_
