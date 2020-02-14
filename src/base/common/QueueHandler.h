#ifndef N_BASE_COMMON_QUEUE_HANDLER_H_
#define N_BASE_COMMON_QUEUE_HANDLER_H_

namespace neptune {
namespace base {

class QueueHandler {
 public:    
  virtual ~QueueHandler() {}
  virtual bool handleQueue(void *data, int len, int threadIndex, void *args) = 0;
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_QUEUE_HANDLER_H_
