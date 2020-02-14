#ifndef N_DFS_UTIL_FILE_QUEUE_THREAD_H
#define N_DFS_UTIL_FILE_QUEUE_THREAD_H

#include "dfs/util/dfs.h"

using namespace neptune::base;

namespace neptune {
namespace dfs {

class CFileQueueThread : public CDefaultRunnable {
 public:
  CFileQueueThread(CFileQueue *queue, int threadCount, QueueHandler *handler, void *args);
  ~CFileQueueThread(void);
  int writeData(void *data, int len);
  void stop();
  void run(CThread *thread, void *args);
  
 private:
  CThreadCond _mutex;
  QueueHandler *_handler;
  void *_args;
  CFileQueue *_queue;
};

} //namespace dfs
} //namespace neptune

#endif
