#ifndef N_DFS_UTIL_META_HASH_HELPER_H_
#define N_DFS_UTIL_META_HASH_HELPER_H_

#include <stdint.h>

namespace neptune {
namespace dfs {

struct HashHelper
{
  HashHelper(const int64_t app_id, const int64_t uid);
  int64_t app_id_;
  int64_t uid_;
};

} //namespace dfs
} //namespace neptune
#endif
