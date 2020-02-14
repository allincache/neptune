#ifndef N_BASE_COMMON_QUEUE_THREAD_H
#define N_BASE_COMMON_QUEUE_THREAD_H

#include <queue>

namespace neptune {
namespace base {
	
class CQueueThread : public CDefaultRunnable {
 public:
  CQueueThread(int threadCount, QueueHandler *handler, void *args);
  ~CQueueThread(void);
  int writeData(void *data, int len);
  void stop();
  void run(CThread *thread, void *args);
            
 private:
  typedef struct data_pair {
    char *data;
    int len;
  } data_pair;
  // queue
  std::queue<data_pair*> _queue;            
    
 protected:
  CThreadCond _mutex;
  QueueHandler *_handler;
  void *_args;
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_QUEUE_THREAD_H

