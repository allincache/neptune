#include <string>
#include "base/concurrent/new/Lock.h"
#include "base/concurrent/new/LockImpl.h"

namespace base {
namespace internal {

namespace {


std::string SystemErrorCodeToString(int error_code) {
  return std::string();
}

}  // namespace


#define PRIORITY_INHERITANCE_LOCKS_POSSIBLE() 1

LockImpl::LockImpl() {
  pthread_mutexattr_t mta;
  int rv = pthread_mutexattr_init(&mta);
  //DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
  rv = pthread_mutex_init(&native_handle_, &mta);
  //DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
  rv = pthread_mutexattr_destroy(&mta);
  //DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
}

LockImpl::~LockImpl() {
  int rv = pthread_mutex_destroy(&native_handle_);
  //DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
}

bool LockImpl::Try() {
  int rv = pthread_mutex_trylock(&native_handle_);
  //DCHECK(rv == 0 || rv == EBUSY) << ". " << SystemErrorCodeToString(rv);
  return rv == 0;
}

void LockImpl::Lock() {
  //if (base::debug::GlobalActivityTracker::IsEnabled())
    if (Try())
      return;

  //base::debug::ScopedLockAcquireActivity lock_activity(this);
  int rv = pthread_mutex_lock(&native_handle_);
  //DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
}


}  // namespace internal
}  // namespace base
