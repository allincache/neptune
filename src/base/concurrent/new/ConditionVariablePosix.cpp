#include <errno.h>
#include <stdint.h>
#include <sys/time.h>

#include "base/concurrent/new/Lock.h"
#include "base/concurrent/new/ConditionVariable.h"

namespace base {

ConditionVariable::ConditionVariable(Lock* user_lock)
    : user_mutex_(user_lock->lock_.native_handle()), user_lock_(user_lock)
{
  int rv = 0;
  rv = pthread_cond_init(&condition_, NULL);
//  DCHECK_EQ(0, rv);
}

ConditionVariable::~ConditionVariable() {
  int rv = pthread_cond_destroy(&condition_);
//  DCHECK_EQ(0, rv);
}

void ConditionVariable::Wait() {
  // Optional<internal::ScopedBlockingCallWithBaseSyncPrimitives>
  //     scoped_blocking_call;
  // if (waiting_is_blocking_)
  //   scoped_blocking_call.emplace(BlockingType::MAY_BLOCK);
  int rv = pthread_cond_wait(&condition_, user_mutex_);
//  DCHECK_EQ(0, rv);
}

// void ConditionVariable::TimedWait(const TimeDelta& max_time) {
//   // Optional<internal::ScopedBlockingCallWithBaseSyncPrimitives>
//   //     scoped_blocking_call;
//   // if (waiting_is_blocking_)
//   //   scoped_blocking_call.emplace(BlockingType::MAY_BLOCK);

//   int64_t usecs = max_time.InMicroseconds();
//   struct timespec relative_time;
//   relative_time.tv_sec = usecs / Time::kMicrosecondsPerSecond;
//   relative_time.tv_nsec =
//       (usecs % Time::kMicrosecondsPerSecond) * Time::kNanosecondsPerMicrosecond;

//   // The timeout argument to pthread_cond_timedwait is in absolute time.
//   struct timespec absolute_time;
//   struct timespec now;
//   clock_gettime(CLOCK_MONOTONIC, &now);
//   absolute_time.tv_sec = now.tv_sec;
//   absolute_time.tv_nsec = now.tv_nsec;

//   absolute_time.tv_sec += relative_time.tv_sec;
//   absolute_time.tv_nsec += relative_time.tv_nsec;
//   absolute_time.tv_sec += absolute_time.tv_nsec / Time::kNanosecondsPerSecond;
//   absolute_time.tv_nsec %= Time::kNanosecondsPerSecond;
// //  DCHECK_GE(absolute_time.tv_sec, now.tv_sec);  // Overflow paranoia

//   int rv = pthread_cond_timedwait(&condition_, user_mutex_, &absolute_time);

//   // On failure, we only expect the CV to timeout. Any other error value means
//   // that we've unexpectedly woken up.
// //  DCHECK(rv == 0 || rv == ETIMEDOUT);
// }

void ConditionVariable::Broadcast() {
  int rv = pthread_cond_broadcast(&condition_);
//  DCHECK_EQ(0, rv);
}

void ConditionVariable::Signal() {
  int rv = pthread_cond_signal(&condition_);
//  DCHECK_EQ(0, rv);
}

}  // namespace base
