#ifndef N_DFS_CLIENT_NAMEMETA_DFS_FILE_MANAGER_H_
#define N_DFS_CLIENT_NAMEMETA_DFS_FILE_MANAGER_H_

#include <string>
#include "dfs/util/meta_server_define.h"
#include "dfs_client_impl.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

const int64_t MAX_WRITE_DATA_IO = 1 << 19;

class DfsMetaManager
{
 public:
  int initialize();
  int destroy();
  int32_t get_cluster_id(const char* ns_addr);
  int64_t read_data(const char* ns_addr, const uint32_t block_id, const uint64_t file_id,
      void* buffer, const int32_t pos, const int64_t length);

  int64_t write_data(const char* ns_addr, const void* buffer, const int64_t pos, const int64_t length,
      FragMeta& frag_meta);
};

}
}

#endif  // N_DFS_CLIENT_RCHELPER_H_
