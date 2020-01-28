#ifndef NEP_BASE_CONCURRENT_LOCKGUARD_H
#define NEP_BASE_CONCURRENT_LOCKGUARD_H

namespace neptune {
namespace base {

template <class T>
class CLockGuard
{

 public:
  CLockGuard(const T& lock, bool block = true) : _lock(lock)
  {
    _acquired = !(block ? _lock.lock() : _lock.tryLock());
  }

  ~CLockGuard()
  {
    _lock.unlock();
  }

  bool acquired() const
  {
    return _acquired;
  }

 private:
  const T& _lock;
  mutable bool _acquired;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_CONCURRENT_LOCKGUARD_H
