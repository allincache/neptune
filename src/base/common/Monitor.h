#ifndef NEP_BASE_COMMON_MONITOR_H
#define NEP_BASE_COMMON_MONITOR_H

#include <iostream>
#include "base/concurrent/Lock.h"
#include "base/concurrent/Cond.h"

namespace neptune {
namespace base {

template <class T>
class Monitor
{
 
 public:
  typedef LockT<Monitor<T> > Lock;
  typedef TryLockT<Monitor<T> > TryLock;

  Monitor();
  ~Monitor();

  void lock() const;
  void unlock() const;
  bool tryLock() const;
  bool wait() const;
  bool timedWait(const Time&) const;
  void notify();
  void notifyAll();

 private:

  Monitor(const Monitor&);
  Monitor& operator=(const Monitor&);
  void notifyImpl(int) const;
  mutable Cond _cond;
  T _mutex;
  mutable int _nnotify;
};

template <class T> 
Monitor<T>::Monitor() :
  _nnotify(0)
{
}

template <class T> 
Monitor<T>::~Monitor()
{
}

template <class T> inline void
Monitor<T>::lock() const
{
  _mutex.lock();
  if(_mutex.willUnlock())
  {
    _nnotify = 0;
  }
}

template <class T> inline void
Monitor<T>::unlock() const
{
  if(_mutex.willUnlock())
  {
    notifyImpl(_nnotify);
  }
  _mutex.unlock();
}

template <class T> inline bool
Monitor<T>::tryLock() const
{
  bool result = _mutex.tryLock();
  if(result && _mutex.willUnlock())
  {
    _nnotify = 0;
  }
  return result;
}

template <class T> inline bool 
Monitor<T>::wait() const
{
  notifyImpl(_nnotify);
#ifdef _NO_EXCEPTION
  const bool bRet = _cond.waitImpl(_mutex);
  _nnotify = 0;
  return bRet;
#else
  try
  {
    _cond.waitImpl(_mutex);
  }
  catch(...)
  {
    _nnotify = 0;
    throw;
  }

  _nnotify = 0;
#endif
  return true;
}

template <class T> inline bool
Monitor<T>::timedWait(const Time& timeout) const
{
  notifyImpl(_nnotify);
#ifdef _NO_EXCEPTION
  const bool rc = _cond.timedWaitImpl(_mutex, timeout);
  _nnotify = 0;
  return rc;
#else
  try
  {
    _cond.timedWaitImpl(_mutex, timeout);
  }
  catch(...)
  {
    _nnotify = 0;
    throw;
  }
  _nnotify = 0;
#endif
  return true;
}

template <class T> inline void
Monitor<T>::notify()
{
  if(_nnotify != -1)
  {
    ++_nnotify;
  }
}

template <class T> inline void
Monitor<T>::notifyAll()
{
  _nnotify = -1;
}

template <class T> inline void
Monitor<T>::notifyImpl(int nnotify) const
{
  if(nnotify != 0)
  {
    if(nnotify == -1)
    {
      _cond.broadcast();
      return;
    }
    else
    {
      while(nnotify > 0)
      {
        _cond.signal();
        --nnotify;
      }
    }
  }
}

} //namespace base
} //namespace neptune
#endif
