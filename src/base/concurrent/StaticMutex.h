#ifndef NEP_BASE_CONCURRENT_STATICMUTEX_H
#define NEP_BASE_CONCURRENT_STATICMUTEX_H

#include "Lock.h"
#include "base/thread/ThreadException.h"

namespace neptune {
namespace base {

class Cond;

class StaticMutex
{
 public:

  typedef LockT<StaticMutex> Lock;
  typedef TryLockT<StaticMutex> TryLock;

  void lock() const;

  bool tryLock() const;

  void unlock() const;

  mutable pthread_mutex_t _mutex;

  friend class Cond;

 private:
  struct LockState
  {
    pthread_mutex_t* mutex;
  };

  void unlock(LockState&) const;
  void lock(LockState&) const;
};

#define TNET_STATIC_MUTEX_INITIALIZER { PTHREAD_MUTEX_INITIALIZER }

extern StaticMutex globalMutex;

inline void StaticMutex::lock() const
{
  const int rt = pthread_mutex_lock(&_mutex);
  if( 0 != rt)
  {
    if(rt == EDEADLK)
    {
#ifdef _NO_EXCEPTION
      if ( rt != 0 )
      {
        assert(!"ThreadLockedException");
        LOG(ERROR,"%s","ThreadLockedException");
      }
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    else
    {
#ifdef _NO_EXCEPTION
      if ( rt != 0 )
      { 
        assert(!"ThreadSyscallException");
        LOG(ERROR,"%s","ThreadSyscallException");
      }
#else
      throw ThreadSyscallException(__FILE__, __LINE__, rt);
#endif
    }
  }
}

inline bool StaticMutex::tryLock() const
{
  const int rc = pthread_mutex_trylock(&_mutex);
  if(rc != 0 && rc != EBUSY)
  {
    if(rc == EDEADLK)
    {
#ifdef _NO_EXCEPTION
      if ( rc != 0 )
      {
        assert(!"ThreadLockedException");
        LOG(ERROR,"%s","ThreadLockedException");
      }
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    else
    {
#ifdef _NO_EXCEPTION
      if ( rc != 0 )
      {
        assert(!"ThreadSyscallException");
        LOG(ERROR,"%s","ThreadSyscallException");
      }
#else
      throw ThreadSyscallException(__FILE__, __LINE__,rc);
#endif
    }
  }
  return (rc == 0);
}

inline void StaticMutex::unlock() const
{
  const int rc = pthread_mutex_unlock(&_mutex);
#ifdef _NO_EXCEPTION
  if ( rc != 0 )
  {
    assert(!"ThreadSyscallException");
    LOG(ERROR,"%s","ThreadSyscallException");
  }
#else
  if(rc != 0)
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rc);
  }
#endif
}

inline void
StaticMutex::unlock(LockState& state) const
{
  state.mutex = &_mutex;
}

inline void
StaticMutex::lock(LockState&) const
{
}

} //namespace base
} //namespace neptune

#endif //NEP_BASE_CONCURRENT_STATICMUTEX_H
