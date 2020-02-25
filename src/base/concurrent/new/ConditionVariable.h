#ifndef BASE_SYNCHRONIZATION_CONDITION_VARIABLE_H_
#define BASE_SYNCHRONIZATION_CONDITION_VARIABLE_H_

#include <pthread.h>
#include "base/concurrent/new/Lock.h"


namespace base {

//class TimeDelta;

class ConditionVariable {
 public:
  explicit ConditionVariable(Lock* user_lock);

  ~ConditionVariable();

  void Wait();
  //void TimedWait(const TimeDelta& max_time);

  void Broadcast();

  void Signal();

  void declare_only_used_while_idle() { waiting_is_blocking_ = false; }

 private:
  pthread_cond_t condition_;
  pthread_mutex_t* user_mutex_;
  base::Lock* const user_lock_;
  bool waiting_is_blocking_ = true;

  //DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
};

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_CONDITION_VARIABLE_H_
