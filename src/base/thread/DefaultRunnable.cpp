#include "base/include/Base.h"

namespace neptune {
namespace base {

CDefaultRunnable::CDefaultRunnable(int threadCount) {
  _stop = false;
  _threadCount = threadCount;
  _thread = NULL;
}

CDefaultRunnable::~CDefaultRunnable() {
  if (_thread) {
    delete[] _thread;
    _thread = NULL;
  }
}

void CDefaultRunnable::setThreadCount(int threadCount)
{
  if (_thread != NULL) {
    return;
  }
  _threadCount = threadCount;
}

// start
int CDefaultRunnable::start() {
  if (_thread != NULL || _threadCount < 1) {
    LOG(ERROR, "start failure, _thread: %p, threadCount: %d", _thread, _threadCount);
    return 0;
  }
  _thread = new CThread[_threadCount];
  if (NULL == _thread)
  {
    LOG(ERROR, "create _thread object failed, threadCount: %d", _threadCount);
    return 0;
  }
  int i = 0;
  for (; i<_threadCount; i++)
  {
    if (!_thread[i].start(this, (void*)((long)i)))
    {
      return i;
    }
  }
  return i;
}

// stop
void CDefaultRunnable::stop() {
  _stop = true;
}

// wait
void CDefaultRunnable::wait() {
  if (_thread != NULL)
  {
    for (int i=0; i<_threadCount; i++)
    {
      _thread[i].join();
    }
  }
}

} //namespace base
} //namespace neptune


