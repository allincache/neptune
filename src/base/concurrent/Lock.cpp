#include "Lock.h"

namespace neptune {
namespace base {

RWLock::RWLock(ELockMode lockMode)
{
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);
  if (lockMode == READ_PRIORITY)
  {
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_READER_NP);
  }
  else if (lockMode == WRITE_PRIORITY)
  {
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
  }
  pthread_rwlock_init(&rwlock_, &attr);
}

RWLock::~RWLock()
{
  pthread_rwlock_destroy(&rwlock_);
}

int RWLock::rdlock()
{
  return pthread_rwlock_rdlock(&rwlock_);
}

int RWLock::wrlock()
{
  return pthread_rwlock_wrlock(&rwlock_);
}

int RWLock::tryrdlock()
{
  return pthread_rwlock_tryrdlock(&rwlock_);
}

int RWLock::trywrlock()
{
  return pthread_rwlock_trywrlock(&rwlock_);
}

int RWLock::unlock()
{
  return pthread_rwlock_unlock(&rwlock_);
}

} // namespace base
} // namespace neptune
