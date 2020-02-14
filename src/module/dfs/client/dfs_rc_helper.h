#ifndef N_DFS_CLIENT_RCHELPER_H_
#define N_DFS_CLIENT_RCHELPER_H_

#include <string>
#include "dfs/util/rc_define.h"

namespace neptune {
namespace dfs {

class RcHelper
{
 public:
  static int login(const uint64_t rc_ip, const std::string& app_key, const uint64_t app_ip,
      std::string& session_id, BaseInfo& base_info);
  static int keep_alive(const uint64_t rc_ip, const KeepAliveInfo& ka_info,
      bool& update_flag, BaseInfo& base_info);
  static int logout(const uint64_t rc_ip, const KeepAliveInfo& ka_info);
  static int reload(const uint64_t rc_ip, const ReloadType reload_type);

 private:

};

}
}

#endif  // N_DFS_CLIENT_RCHELPER_H_
