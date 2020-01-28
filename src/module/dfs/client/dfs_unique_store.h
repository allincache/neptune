#ifndef N_DFS_CLIENT_DFSUNIQUESTORE_H_
#define N_DFS_CLIENT_DFSUNIQUESTORE_H_

#include "base/common/Internal.h"
#include "tair_unique_handler.h"

namespace neptune {
namespace dfs {

enum UniqueAction
{
  UNIQUE_ACTION_NONE = 0,
  UNIQUE_ACTION_SAVE_DATA,
  UNIQUE_ACTION_SAVE_DATA_SAVE_META,
  UNIQUE_ACTION_SAVE_DATA_UPDATE_META,
  UNIQUE_ACTION_UPDATE_META
};

const static int32_t UNIQUE_FIRST_INSERT_VERSION = 0x0FFFFFFF;

class DfsUniqueStore
{
 public:
  DfsUniqueStore();
  ~DfsUniqueStore();

  int initialize(const char* master_addr, const char* slave_addr, const char* group_name, const int32_t area,
                  const char* ns_addr);

  int64_t save(const char* buf, const int64_t count,
                const char* dfs_name, const char* suffix,
                char* ret_dfs_name, const int32_t ret_dfs_name_len);

  int64_t save(const char* local_file,
                const char* dfs_name, const char* suffix,
                char* ret_dfs_name, const int32_t ret_dfs_name_len);

  int32_t unlink(const char* dfs_name, const char* suffix, int64_t& file_size, const int32_t count);

private:
  bool check_init();
  bool check_name_match(const char* orig_dfs_name, const char* dfs_name, const char* suffix);
  bool check_suffix_match(const char* orig_dfs_name, const char* suffix);
  bool check_dfsname_match(const char* orig_dfs_name, const char* dfs_name, const char* suffix);
  UniqueAction check_unique(UniqueKey& unique_key, UniqueValue& unique_value,
                        const char* dfs_name, const char* suffix);
  int process(UniqueAction action, UniqueKey& unique_key, UniqueValue& unique_value,
              const char* dfs_name, const char* suffix,
              char* ret_dfs_name, const int32_t ret_dfs_name_len);

  int save_data(UniqueKey& unique_key,
                const char* dfs_name, const char* suffix,
                char* ret_dfs_name, const int32_t ret_dfs_name_len);
  int save_data_save_meta(UniqueKey& unique_key, UniqueValue& unique_value,
                          const char* dfs_name, const char* suffix,
                          char* ret_dfs_name, const int32_t ret_dfs_name_len);
  int save_data_update_meta(UniqueKey& unique_key, UniqueValue& unique_value,
                            const char* dfs_name, const char* suffix,
                            char* ret_dfs_name, const int32_t ret_dfs_name_len);
  int update_meta(UniqueKey& unique_key, UniqueValue& unique_value,
                  char* ret_dfs_name, const int32_t ret_dfs_name_len);

  int wrap_file_name(const char* dfs_name, char* ret_dfs_name, const int32_t ret_dfs_name_len);
  int64_t get_local_file_size(const char* local_file);
  int read_local_file(const char* local_file, char*& buf, int64_t& count);

 private:
  UniqueHandler<UniqueKey, UniqueValue>* unique_handler_;
  std::string ns_addr_;
};

}
}

#endif
