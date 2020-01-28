#ifndef N_BASE_COMMON_DEFAULT_RUNNABLE_H
#define N_BASE_COMMON_DEFAULT_RUNNABLE_H

namespace neptune {
namespace base {

class CDefaultRunnable : public Runnable {

 public:
  CDefaultRunnable(int threadCount = 1);
  
  virtual ~CDefaultRunnable();

  void setThreadCount(int threadCount);
  
  int start();

  void stop();

  void wait();

 protected:    
  CThread *_thread;
  int _threadCount;
  bool _stop;
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_DEFAULT_RUNNABLE_H
