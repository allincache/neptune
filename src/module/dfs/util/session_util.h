#ifndef N_DFS_UTIL_SESSIONUTIL_H_
#define N_DFS_UTIL_SESSIONUTIL_H_

#include <string>
#include <stdint.h>

namespace neptune {
namespace dfs {

static const char SEPARATOR_KEY = '-';
class SessionUtil
{
 public:
  static std::string gene_uuid_str();
  static void gene_session_id(const int32_t app_id, const int64_t session_ip, std::string& session_id);
  static int parse_session_id(const std::string& session_id, int32_t& app_id, int64_t& session_ip);
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_UTIL_SESSIONUTIL_H_
