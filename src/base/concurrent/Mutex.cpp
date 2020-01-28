#include "Mutex.h"

namespace neptune {
namespace base {

Mutex::Mutex()
{
  const int rt = pthread_mutex_init(&_mutex, NULL);
#ifdef _NO_EXCEPTION
  assert( rt == 0 );
  if ( rt != 0 )
  {
  //    LOG(ERROR,"%s","ThreadSyscallException");
  } 
#else
  if ( rt != 0 )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
}

Mutex::~Mutex()
{
  const int rt = pthread_mutex_destroy(&_mutex); 
  assert(rt == 0);
  if ( rt != 0 )
  {
  //    LOG(ERROR,"%s","ThreadSyscallException");
  } 
}

void Mutex::lock() const
{
  const int rt = pthread_mutex_lock(&_mutex);
#ifdef _NO_EXCEPTION
  assert( rt == 0 );
  if ( rt != 0 )
  {
    if ( rt == EDEADLK )
    {
    //    LOG(ERROR,"%s","ThreadLockedException "); 
    }
    else
    {
    //    LOG(ERROR,"%s","ThreadSyscallException"); 
    }
  }
#else
  if( rt != 0 )
  {
    if(rt == EDEADLK)
    {
      throw ThreadLockedException(__FILE__, __LINE__);
    }
    else
    {
      throw ThreadSyscallException(__FILE__, __LINE__, rt);
    }
  }
#endif
}

bool Mutex::tryLock() const
{
  const int rt = pthread_mutex_trylock(&_mutex);
#ifdef _NO_EXCEPTION
  if ( rt != 0 && rt !=EBUSY )
  {
    if ( rt == EDEADLK )
    {
    //    LOG(ERROR,"%s","ThreadLockedException "); 
    }
    else
    {
    //    LOG(ERROR,"%s","ThreadSyscallException"); 
    }
    return false;
  }
#else
  if(rt != 0 && rt != EBUSY)
  {
    if(rt == EDEADLK)
    {
      throw ThreadLockedException(__FILE__, __LINE__);
    }
    else
    {
      throw ThreadSyscallException(__FILE__, __LINE__, rt);
    }
  }
#endif
  return (rt == 0);
}

void Mutex::unlock() const
{
  const int rt = pthread_mutex_unlock(&_mutex);
#ifdef _NO_EXCEPTION
  assert( rt == 0 );
  if ( rt != 0 )
  {
  //    LOG(ERROR,"%s","ThreadSyscallException");
  } 
#else
  if ( rt != 0 )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
}

void Mutex::unlock(LockState& state) const
{
  state.mutex = &_mutex;
}

void Mutex::lock(LockState&) const
{
}

bool Mutex::willUnlock() const
{
  return true;
}

} //namespace base
} //namespace neptune
