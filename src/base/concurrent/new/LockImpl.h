#ifndef BASE_SYNCHRONIZATION_LOCK_IMPL_H_
#define BASE_SYNCHRONIZATION_LOCK_IMPL_H_

#include <errno.h>
#include <pthread.h>

namespace base {
namespace internal {

class LockImpl {
 public:
  using NativeHandle = pthread_mutex_t;

  LockImpl();
  ~LockImpl();

  bool Try();

  void Lock();

  inline void Unlock();

  NativeHandle* native_handle() { return &native_handle_; }

  static bool PriorityInheritanceAvailable();

 private:
  NativeHandle native_handle_;
  //DISALLOW_COPY_AND_ASSIGN(LockImpl);
};


void LockImpl::Unlock() {
  int rv = pthread_mutex_unlock(&native_handle_);
  //DCHECK_EQ(rv, 0) << ". " << strerror(rv);
}

}  // namespace internal
}  // namespace base

#endif  // BASE_SYNCHRONIZATION_LOCK_IMPL_H_
