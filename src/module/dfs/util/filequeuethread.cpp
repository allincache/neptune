#include "dfs/util/dfs.h"

namespace neptune {
namespace dfs {

CFileQueueThread::CFileQueueThread(CFileQueue *queue, int threadCount, QueueHandler *handler, void *args) : 
  CDefaultRunnable(threadCount)
{
  assert(queue != NULL);
  _queue = queue;
  _handler = handler;
  _args = args;
}

CFileQueueThread::~CFileQueueThread(void)
{
  stop();
}

int CFileQueueThread::writeData(void *data, int len)
{
  if (data == NULL || len <= 0) {
      return EXIT_FAILURE;
  }
  _mutex.lock();
  _queue->push(data, len);
  _mutex.signal();
  _mutex.unlock();
  return EXIT_SUCCESS;
}

void CFileQueueThread::stop()
{
  _mutex.lock();
  _stop = true;
  _mutex.broadcast();
  _mutex.unlock();
}

void CFileQueueThread::run(CThread *thread, void *args)
{
  int threadIndex = (int)((long)(args));
  int total_count = 0;
  int64_t end_time = 0;
  int64_t start_time = CTimeUtil::getTime();
  bool dosuc = true;
  _mutex.lock();
  while(!_stop) {
      while(_stop == 0 && _queue->isEmpty()) {
          _mutex.wait();
      }
      if (_stop) {
          break;
      }
      queue_item *item = _queue->pop(threadIndex);
      int cnt = ++total_count;
      if (total_count >= 1000) {
          total_count = 0;
          end_time = CTimeUtil::getTime();
      }
      _mutex.unlock();
      if (item != NULL) {
          if (_handler != NULL) {
              _handler->handleQueue(&item->data[0], item->len, threadIndex, _args);
          }
          dosuc = true;
          free(item);
      } else {
          dosuc = false;
      }
      if (end_time > start_time) {
          //LOG(INFO, "task execute speed: %d task/s", (int)((int64_t)1000000*cnt/(end_time-start_time)));
          start_time = end_time;
      }
      _mutex.lock();
      if (dosuc) _queue->finish(threadIndex);
  }
  _mutex.unlock();
}

} //namespace dfs
} //namespace neptune

