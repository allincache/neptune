#ifndef N_DFS_CLIENT_NAMEMETA_HELPER_H_
#define N_DFS_CLIENT_NAMEMETA_HELPER_H_

#include <string>
#include "dfs/util/meta_server_define.h"
#include "base/common/Internal.h"

namespace neptune {
namespace dfs {

class NameMetaHelper
{
 public:
  static int do_file_action(const uint64_t server_id, const int64_t app_id, const int64_t user_id,
      const MetaActionOp action, const char* path, const char* new_path, const int64_t version_id);

  static int do_write_file(const uint64_t server_id, const int64_t app_id, const int64_t user_id,
      const char* path, const int64_t version_id, const FragInfo& frag_info);

  static int do_read_file(const uint64_t server_id, const int64_t app_id, const int64_t user_id,
      const char* path, const int64_t offset, const int64_t size, const int64_t version_id,
      FragInfo& frag_info, bool& still_have);

  static int do_ls(const uint64_t server_id, const int64_t app_id, const int64_t user_id,
      const char* path, const FileType file_type, const int64_t pid, const int64_t version_id,
      std::vector<MetaInfo>& meta_info, bool& still_have);
  static int get_table(const uint64_t server_id,
      char* table_info, uint64_t& table_length, int64_t& version_id);
};

}
}

#endif  // N_DFS_CLIENT_RCHELPER_H_
