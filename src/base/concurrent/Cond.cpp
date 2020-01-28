#include <sys/time.h>
#include "Cond.h"

namespace neptune {
namespace base {

Cond::Cond()
{
  pthread_condattr_t attr;
  int rt = pthread_condattr_init(&attr);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
#else
  if( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
  
  rt =  pthread_cond_init(&_cond, &attr);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
#else
  if( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif

  rt = pthread_condattr_destroy(&attr);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
#else
  if( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
}

Cond::~Cond()
{
  pthread_cond_destroy(&_cond);
}

void Cond::signal()
{
  const int rt = pthread_cond_signal(&_cond);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
#else
  if ( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
}

void Cond::broadcast()
{
  const int rt = pthread_cond_broadcast(&_cond);
#ifdef _NO_EXCEPTION
  assert( 0 == rt );
#else
  if( 0 != rt )
  {
    throw ThreadSyscallException(__FILE__, __LINE__, rt);
  }
#endif
}

} //namespace base
} //namespace neptune
