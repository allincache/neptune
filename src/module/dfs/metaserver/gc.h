#ifndef N_DFS_MS_GC_H_
#define N_DFS_MS_GC_H_

#include "base/time/Timer.h"
#include "base/concurrent/Mutex.h"
#include "ns_define.h"

#ifdef NEP_GTEST
#include <gtest/gtest.h>
#endif

namespace neptune {
namespace dfs {
namespace metaserver {

class LayoutManager;
class GCObjectManager
{
  #ifdef NEP_GTEST
  friend class GCTest;
  friend class LayoutManager;
  FRIEND_TEST(GCTest, add);
  FRIEND_TEST(GCTest, gc);
  #endif
 
 public:
  explicit GCObjectManager(LayoutManager& manager);
  virtual ~GCObjectManager();
  int add(GCObject* object, const time_t now = Func::get_monotonic_time());
  int gc(const time_t now);
  int64_t size() const;
 
 private:
  DISALLOW_COPY_AND_ASSIGN(GCObjectManager);
  LayoutManager& manager_;
  std::set<GCObject*>  wait_clear_list_;
  std::list<GCObject*> wait_free_list_;
  Mutex mutex_;
  int64_t wait_free_list_size_;
};

}
}
}

#endif
