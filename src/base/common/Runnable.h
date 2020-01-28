#ifndef N_BASE_COMMON_RUNNABLE_H
#define N_BASE_COMMON_RUNNABLE_H

namespace neptune {
namespace base {

class Runnable {

 public:

  virtual ~Runnable() {}
  
  virtual void run(CThread *thread, void *arg) = 0;
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_RUNNABLE_H
