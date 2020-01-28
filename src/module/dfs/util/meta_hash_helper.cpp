#include "meta_hash_helper.h"
#include "base/common/Serialization.h"
#include "dfs/util/dfs.h"

namespace neptune {
namespace dfs {

HashHelper::HashHelper(const int64_t app_id, const int64_t uid)
{
  assert(SUCCESS == Serialization::int64_to_char((char*)(&app_id_), sizeof(int64_t), app_id));
  assert(SUCCESS == Serialization::int64_to_char((char*)(&uid_), sizeof(int64_t), uid));
}

} //namespace dfs
} //namespace neptune
