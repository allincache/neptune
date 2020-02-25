#include "base/concurrent/new/Lock.h"


namespace base {

Lock::Lock() : lock_() {
}

Lock::~Lock() {
  //DCHECK(owning_thread_ref_.is_null());
}

void Lock::AssertAcquired() const {
  //DCHECK(owning_thread_ref_ == PosixThread::CurrentRef());
}

void Lock::CheckHeldAndUnmark() {
  //DCHECK(owning_thread_ref_ == PosixThread::CurrentRef());
  owning_thread_ref_ = PosixThreadRef();
}

void Lock::CheckUnheldAndMark() {
  //DCHECK(owning_thread_ref_.is_null());
  owning_thread_ref_ = PosixThread::CurrentRef();
}

}  // namespace base

