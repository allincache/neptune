#ifndef N_DFS_MS_GLOBAL_FACTORY_H_
#define N_DFS_MS_GLOBAL_FACTORY_H_

#include <string>
#include "base/time/Timer.h"
#include "gc.h"
#include "ns_define.h"
#include "dfs/util/statistics.h"

namespace neptune {
namespace dfs {
namespace metaserver {

struct GFactory
{
  static int initialize(TimerPtr timer);
  static int wait_for_shut_down();
  static int destroy();
  static NsRuntimeGlobalInformation& get_runtime_info()
  {
    return NsRuntimeGlobalInformation::instance();
  }
  static NsGlobalStatisticsInfo& get_global_info()
  {
    return NsGlobalStatisticsInfo::instance();
  }
  static StatManager<std::string, std::string, StatEntry >& get_stat_mgr()
  {
    return stat_mgr_;
  }
  static StatManager<std::string, std::string, StatEntry > stat_mgr_;
  static std::string dfs_ns_stat_;
  //static std::string dfs_ns_stat_block_count_;
};

}
}
}

#endif


