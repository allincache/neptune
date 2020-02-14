#ifndef DFS_BG_TASK_H_
#define DFS_BG_TASK_H_

#include <string>
#include "base/time/Timer.h"
#include "dfs/util/statistics.h"
#include "gc_worker.h"

namespace neptune {
namespace dfs {

class BgTask
{
 public:
  static int initialize();
  static int wait_for_shut_down();
  static int destroy();
  static int32_t get_cache_hit_ratio(CacheType cache_type = LOCAL_CACHE);

  static StatManager<std::string, std::string, StatEntry>& get_stat_mgr()
  {
    return stat_mgr_;
  }
  static GcManager& get_gc_mgr()
  {
    return gc_mgr_;
  }

 private:
  static TimerPtr timer_;
  static StatManager<std::string, std::string, StatEntry> stat_mgr_;
  static GcManager gc_mgr_;
};

}
}

#endif
