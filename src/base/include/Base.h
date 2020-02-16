#ifndef NEPTUNE_BASE_H
#define NEPTUNE_BASE_H

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif


namespace neptune {
namespace base {

class Buffer;
class CConfig;
class CFileUtil;
class CNetUtil;
class CTimeUtil;
class CProcess;
class CLogger;
class CThread;
class CThreadMutex;
class CThreadCond; 
class Runnable;
class CDefaultRunnable;
class QueueHandler;
class CQueueThread;
class Exception;


class noncopyable
{
 protected:
  noncopyable() {}
  ~noncopyable() {}

 private:
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);
};

#if defined(__BCPLUSPLUS__) || defined(_MSC_VER)
  typedef __int64 Int64;
  #define T_INT64(n) n##i64
#elif defined(TNET_64)
  typedef long Int64;
  #define T_INT64(n) n##L
#else
  typedef long long Int64;
  #define T_INT64(n) n##LL
#endif
 
typedef unsigned char Byte;
typedef short Short;
typedef int Int;
typedef Int64 Long;
typedef float Float;
typedef double Double;
 
typedef ::std::vector<bool> BoolSeq;
typedef ::std::vector<Byte> ByteSeq;
typedef ::std::vector<Short> ShortSeq;
typedef ::std::vector<Int> IntSeq;
typedef ::std::vector<Long> LongSeq;
typedef ::std::vector<Float> FloatSeq;
typedef ::std::vector<Double> DoubleSeq;
typedef ::std::vector<::std::string> StringSeq;
 
inline int getSystemErrno()
{
  return errno;
}

} //namespace neptune
} //namespace base

#include "base/common/Buffer.h"
#include "base/common/Config.h"
#include "base/fs/FileUtil.h"
#include "base/network/NetworkUtil.h"
#include "base/time/TimeUtil.h"
#include "base/common/Atomic.h"
#include "base/concurrent/RwLock.h"
#include "base/thread/Runnable.h"
#include "base/thread/QueueHandler.h"
#include "base/thread/DefaultRunnable.h"
#include "base/process/Process.h"
#include "base/log/Log.h"
#include "base/thread/CThread.h"
#include "base/thread/CThreadMutex.h"
#include "base/thread/CThreadCond.h"
#include "base/thread/QueueThread.h"
#include "base/thread/ThreadLocal.h"
#include "base/common/Exception.h"
#include "base/common/Serialization.h"


#endif //NEPTUNE_BASE_H