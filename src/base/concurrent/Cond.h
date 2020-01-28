#ifndef NEP_BASE_CONCURRENT_COND_H
#define NEP_BASE_CONCURRENT_COND_H

#include "base/include/Base.h"
#include "base/time/Time.h"
#include "base/thread/ThreadException.h"

namespace neptune {
namespace base {

template<class T> class Monitor;
class Mutex;
class RecMutex;

class Cond : private noncopyable
{
 public:

  Cond();
  ~Cond();

  void signal();

  void broadcast();

  template <typename Lock> inline bool 
  wait(const Lock& lock) const
  {
    if(!lock.acquired())
    {
#ifdef _NO_EXCEPTION
      LOG(ERROR,"%s","ThreadLockedException");
      return false;
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    return waitImpl(lock._mutex);
  }

  template <typename Lock> inline bool
  timedWait(const Lock& lock, const Time& timeout) const
  {
    if(!lock.acquired())
    {
#ifdef _NO_EXCEPTION
      LOG(ERROR,"%s","ThreadLockedException");
      return false;
#else
      throw ThreadLockedException(__FILE__, __LINE__);
#endif
    }
    return timedWaitImpl(lock._mutex, timeout);
  }

 private:

  friend class Monitor<Mutex>;
  friend class Monitor<RecMutex>;

  template <typename M> bool waitImpl(const M&) const;
  template <typename M> bool timedWaitImpl(const M&, const Time&) const;

  mutable pthread_cond_t _cond;
  };

  template <typename M> inline bool 
  Cond::waitImpl(const M& mutex) const
  {
    typedef typename M::LockState LockState;

    LockState state;
    mutex.unlock(state);
    const int rc = pthread_cond_wait(&_cond, state.mutex);
    mutex.lock(state);

    #ifdef _NO_EXCEPTION
    if( 0 != rc )
    {
      LOG(ERROR,"%s","ThreadSyscallException");
      return false;
    } 
    #else
    if(0 != rc)
    {
      throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    #endif
    return true;
  }

  template <typename M> inline bool
  Cond::timedWaitImpl(const M& mutex, const Time& timeout) const
  {
    if(timeout < Time::microSeconds(0))
    {
    #ifdef _NO_EXCEPTION
      LOG(ERROR,"%s","InvalidTimeoutException");
      return false;
    #else
      throw InvalidTimeoutException(__FILE__, __LINE__, timeout);
    #endif
  }

  typedef typename M::LockState LockState;

  LockState state;
  mutex.unlock(state);

  timeval tv = Time::now(Time::Realtime) + timeout;
  timespec ts;
  ts.tv_sec = tv.tv_sec;
  ts.tv_nsec = tv.tv_usec * 1000;
  /*timeval tv = Time::now(Time::Realtime);
  timespec ts;
  ts.tv_sec  = tv.tv_sec + timeout/1000;
  ts.tv_nsec = tv.tv_usec * 1000 + ( timeout % 1000 ) * 1000000;*/
  const int rc = pthread_cond_timedwait(&_cond, state.mutex, &ts);
  mutex.lock(state);

  if(rc != 0)
  {
  #ifdef _NO_EXCEPTION
    if ( rc != ETIMEDOUT )
    {
      LOG(ERROR,"%s","ThreadSyscallException");
      return false;
    }
#else
    if(rc != ETIMEDOUT)
    {
      throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
  #endif
  }
  return true;
}

}// namespace base
}// namespace neptune

#endif
