#include "CountMutex.h"
#include "base/common/Exception.h"

namespace neptune {
namespace base {

CountMutex::CountMutex() :
  _count(0)
{
  pthread_mutexattr_t attr;
  int rt = pthread_mutexattr_init(&attr);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
  if ( rt != 0 )
  {
      //LOG(ERROR,"%s","ThreadSyscallException");
  }
#else
  if ( 0 != rt ) 
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
  rt = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
  if ( rt != 0 )
  {
      //LOG(ERROR,"%s","ThreadSyscallException");
  }
#else
  if ( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
  rt = pthread_mutex_init(&_mutex, &attr);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
  if ( rt != 0 )
  {
      //LOG(ERROR,"%s","ThreadSyscallException");
  }
#else
  if ( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
  
  rt = pthread_mutexattr_destroy(&attr);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
  if ( rt != 0 )
  {
      //LOG(ERROR,"%s","ThreadSyscallException");
  }
#else
  if ( 0 != rt )
  {
      throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
}

CountMutex::~CountMutex()
{
  assert(_count == 0);
  const int rc = pthread_mutex_destroy(&_mutex);
  assert(rc == 0);
  if ( rc != 0 )
  {
      //LOG(ERROR,"%s","ThreadSyscallException");
  }
}

void CountMutex::lock() const
{
  const int rt = pthread_mutex_lock(&_mutex);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
  if ( rt != 0 )
  {
      //LOG(ERROR,"%s","ThreadSyscallException");
  }
#else
  if(0 != rt)
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
  if(++_count > 1)
  {
    const int rc = pthread_mutex_unlock(&_mutex);
    assert(rc == 0);
  }
}

bool CountMutex::tryLock() const
{
  const int rc = pthread_mutex_trylock(&_mutex);
  const bool result = (rc == 0);
  if(!result)
  {
#ifdef _NO_EXCEPTION
    assert( EBUSY == rc );
#else
    if(rc != EBUSY)
    {
      throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif
  } 
  else if(++_count > 1)
  {
    const int rt = pthread_mutex_unlock(&_mutex);
#ifdef _NO_EXCEPTION
    assert( 0 == rt );
    if ( rt != 0 )
    {
        //LOG(ERROR,"%s","ThreadSyscallException");
    }
#else
    if( 0 != rt)
    {
      throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif
  }
  return result;
}

void CountMutex::unlock() const
{
  if(--_count == 0)
  {
    const int rc = pthread_mutex_unlock(&_mutex);
    assert(rc == 0);
  }
}

void CountMutex::unlock(LockState& state) const
{
  state.mutex = &_mutex;
  state.count = _count;
  _count = 0;
}

void CountMutex::lock(LockState& state) const
{
  _count = state.count;
}

bool CountMutex::willUnlock() const
{
  return _count == 1;
}

} //namespace base
} //namespace neptune
