#include "global_factory.h"
#include "base/common/Internal.h"
#include "base/common/ErrorMsg.h"
#include "server_collect.h"


namespace neptune {
namespace dfs {
namespace metaserver {

NsGlobalStatisticsInfo NsGlobalStatisticsInfo::instance_;
NsRuntimeGlobalInformation NsRuntimeGlobalInformation::instance_;
StatManager<std::string, std::string, StatEntry >GFactory::stat_mgr_;
std::string GFactory::dfs_ns_stat_ = "dfs-ns-stat";
//std::string GFactory::dfs_ns_stat_block_count_ = "dfs-ns-stat-block-count";

int GFactory::initialize(TimerPtr timer)
{
  int32_t ret = stat_mgr_.initialize(timer);
  if (ret != SUCCESS)
  {
    LOG(ERROR, "%s", "initialize stat manager fail");
    return ret;
  }
  int64_t current = CTimeUtil::getTime();
  StatEntry<std::string, std::string>::StatEntryPtr stat_ptr = new StatEntry<std::string, std::string>(dfs_ns_stat_, current, true);
  stat_ptr->add_sub_key("dfs-ns-read-success");
  stat_ptr->add_sub_key("dfs-ns-read-failed");
  stat_ptr->add_sub_key("dfs-ns-write-success");
  stat_ptr->add_sub_key("dfs-ns-write-failed");
  stat_ptr->add_sub_key("dfs-ns-unlink");

  stat_mgr_.add_entry(stat_ptr, SYSPARAM_NAMESERVER.dump_stat_info_interval_);

  /*StatEntry<std::string, std::string>::StatEntryPtr ptr = new StatEntry<std::string, std::string>(dfs_ns_stat_block_count_, current, false);
  ptr->add_sub_key("dfs-ns-block-count");
  stat_mgr_.add_entry(ptr, SYSPARAM_NAMESERVER.dump_stat_info_interval_);*/
  return ret;
}

int GFactory::wait_for_shut_down()
{
  stat_mgr_.wait_for_shut_down();
  return SUCCESS;
}

int GFactory::destroy()
{
  get_runtime_info().destroy();
  stat_mgr_.destroy();
  return SUCCESS;
}

}
}
}
