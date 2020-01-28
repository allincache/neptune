#ifndef NEP_BASE_CONCURRENT_RWLOCK_H
#define NEP_BASE_CONCURRENT_RWLOCK_H

#include <pthread.h>
#include "LockGuard.h"

namespace neptune {
namespace base {

enum ERwLockMode
{
  NO_RW_PRIORITY,
  WRITE_RW_PRIORITY,
  READ_RW_PRIORITY
};

class CRLock
{
 public:
  CRLock(pthread_rwlock_t* lock) : _rlock(lock) {}
  ~CRLock() {}
    
  int lock() const;

  int tryLock() const;

  int unlock() const;
    
 private:
  mutable pthread_rwlock_t* _rlock;
};

class CWLock
{
 public:
  CWLock(pthread_rwlock_t* lock) : _wlock(lock) {}
  ~CWLock() {}
  
  int lock() const;
  int tryLock() const;
  int unlock() const;
    
 private:
  mutable pthread_rwlock_t* _wlock;
};    

class CRWLock 
{
 public:
  CRWLock(ERwLockMode lockMode = NO_RW_PRIORITY);
  ~CRWLock();

  CRLock* rlock() const {return _rlock;}
  CWLock* wlock() const {return _wlock;} 

 private:
  CRLock* _rlock;
  CWLock* _wlock;
  pthread_rwlock_t _rwlock;
};

class CRWSimpleLock
{
 public:
  CRWSimpleLock(ERwLockMode lockMode = NO_RW_PRIORITY);
  ~CRWSimpleLock();
  
  int rdlock();
  int wrlock();
  int tryrdlock();
  int trywrlock();
  int unlock();
  
 private:    
  pthread_rwlock_t _rwlock;
};

class CRLockGuard
{
 public:
  CRLockGuard(const CRWLock& rwlock, bool block = true) : _guard((*rwlock.rlock()), block) {}
  ~CRLockGuard(){}

  bool acquired()
  {
    return _guard.acquired();
  }

 private:
  CLockGuard<CRLock> _guard;
};

class CWLockGuard
{
 public:
  CWLockGuard(const CRWLock& rwlock, bool block = true) : _guard((*rwlock.wlock()), block) {}
  ~CWLockGuard(){}

  bool acquired()
  {
    return _guard.acquired();
  }

 private:
  CLockGuard<CWLock> _guard;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_CONCURRENT_RWLOCK_H
