#ifndef NEP_DFS_D_H
#define NEP_DFS_D_H

#include <assert.h>
#include <errno.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <vector>
#include <string>

#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif

namespace neptune{
namespace dfs{

class CFileQueue;
class CFileQueueThread;
class Stream;

class noncopyable
{
 protected:
  noncopyable() { }
  ~noncopyable() { }
 
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

} //namespace dfs
} //namespace neptune

#include "base/include/Base.h"
#include "dfs/util/filequeue.h"
#include "dfs/util/filequeuethread.h"
#include "dfs/util/stream.h"

#endif

