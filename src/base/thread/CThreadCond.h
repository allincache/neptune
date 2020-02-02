#ifndef NEP_BASE_THREAD_COND_H
#define NEP_BASE_THREAD_COND_H

namespace neptune {
namespace base {

class CThreadCond : public CThreadMutex {

 public:
  CThreadCond() {
    pthread_cond_init(&_cond, NULL);
  }
  
  ~CThreadCond() {
    pthread_cond_destroy(&_cond);
  }

  bool wait(int milliseconds = 0) {
    bool ret = true;
    if (milliseconds == 0) { 
      pthread_cond_wait(&_cond, &_mutex);
    } else {
      struct timeval curtime;
      struct timespec abstime;
      gettimeofday(&curtime, NULL);
      int64_t us = (static_cast<int64_t>(curtime.tv_sec) *
                    static_cast<int64_t>(1000000) +
                    static_cast<int64_t>(curtime.tv_usec) +
                    static_cast<int64_t>(milliseconds) *
                    static_cast<int64_t>(1000));
      abstime.tv_sec = static_cast<int>(us / static_cast<int64_t>(1000000));
      abstime.tv_nsec = static_cast<int>(us % static_cast<int64_t>(1000000)) * 1000;
      ret = (pthread_cond_timedwait(&_cond, &_mutex, &abstime) == 0);
    }
    return ret;
  }

  void signal() {
    pthread_cond_signal(&_cond);
  }

  void broadcast() {
    pthread_cond_broadcast(&_cond);
  }

 private:
  pthread_cond_t _cond;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_THREAD_COND_H
