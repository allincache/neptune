#ifndef NEP_BASE_CONCURRENT_LOCK_H
#define NEP_BASE_CONCURRENT_LOCK_H

#include <pthread.h>
#include "base/thread/ThreadException.h"

namespace neptune {
namespace base {

template <typename T>
class LockT
{
 public:
    
  LockT(const T& mutex) :
    _mutex(mutex)
  {
    _mutex.lock();
    _acquired = true;
  }

  ~LockT()
  {
    if (_acquired)
    {
      _mutex.unlock();
    }
  }
  
  void acquire() const
  {
    if (_acquired)
    {
#ifdef _NO_EXCEPTION
      assert(!"ThreadLockedException");
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    _mutex.lock();
    _acquired = true;
  }

  bool tryAcquire() const
  {
    if (_acquired)
    {
#ifdef _NO_EXCEPTION
      assert(!"ThreadLockedException");
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    _acquired = _mutex.tryLock();
    return _acquired;
  }

  void release() const
  {
    if (!_acquired)
    {
#ifdef _NO_EXCEPTION
      assert(!"ThreadLockedException");
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    _mutex.unlock();
    _acquired = false;
  }

  bool acquired() const
  {
    return _acquired;
  }
  
 protected:
  
  LockT(const T& mutex, bool) :
    _mutex(mutex)
  {
    _acquired = _mutex.tryLock();
  }

 private:
  
  LockT(const LockT&);
  LockT& operator=(const LockT&);

  const T& _mutex;
  mutable bool _acquired;

  friend class Cond;
};

template <typename T>
class TryLockT : public LockT<T>
{
 public:

  TryLockT(const T& mutex) :
    LockT<T>(mutex, true)
  {}
};

enum ELockMode
{
  NO_PRIORITY,
  WRITE_PRIORITY,
  READ_PRIORITY
};

class ScopedRWLock;
class RWLock
{
  public:
    typedef ScopedRWLock Lock;
    RWLock(ELockMode lockMode = NO_PRIORITY);
    virtual ~RWLock();

    int rdlock() ;
    int wrlock() ;
    int tryrdlock() ;
    int trywrlock() ;
    int unlock() ;

  private:
    pthread_rwlock_t rwlock_;
};

enum ELockType
{
  READ_LOCKER,
  WRITE_LOCKER
};

class ScopedRWLock
{
 public:
  ScopedRWLock(RWLock& locker, const ELockType lock_type)
    : locker_(locker)
  {
    if (lock_type == READ_LOCKER)
    {
      int ret = locker_.rdlock();
      if (0 !=ret)
      {
        //LOG(WARN , "lock failed, ret: %d", ret);
      }
    }
    else
    {
      int ret = locker_.wrlock();
      if (0 !=ret)
      {
        //LOG(WARN , "lock failed, ret: %d", ret);
      }
    }
  }

  ~ScopedRWLock()
  {
    int ret = locker_.unlock();
    if (0 !=ret)
    {
      //LOG(WARN , "unlock failed, ret: %d", ret);
    }
  }

  private:
    RWLock& locker_;
};

} //namespace base 
} //namespace neptune

#endif
