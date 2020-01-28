#ifndef N_DFS_CLIENT_META_CLIENT_API_H_
#define N_DFS_CLIENT_META_CLIENT_API_H_

#include <string>
#include "base/common/Define.h"
#include "dfs/util/stream.h"
#include "dfs/util/meta_server_define.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

typedef int DfsRetType;
class NameMetaClientImpl;
class NameMetaClient
{
 public:
  NameMetaClient();
  ~NameMetaClient();

  int initialize(const char* rs_addr);
  int initialize(const int64_t rs_addr);

  DfsRetType create_dir(const int64_t app_id, const int64_t uid, const char* dir_path);
  DfsRetType create_dir_with_parents(const int64_t app_id, const int64_t uid, const char* dir_path);
  DfsRetType create_file(const int64_t app_id, const int64_t uid, const char* file_path);

  DfsRetType rm_dir(const int64_t app_id, const int64_t uid, const char* dir_path);
  DfsRetType rm_file(const char* ns_addr, const int64_t app_id, const int64_t uid, const char* file_path);

  DfsRetType mv_dir(const int64_t app_id, const int64_t uid,
      const char* src_dir_path, const char* dest_dir_path);
  DfsRetType mv_file(const int64_t app_id, const int64_t uid,
      const char* src_file_path, const char* dest_file_path);

  DfsRetType ls_dir(const int64_t app_id, const int64_t uid,
      const char* dir_path,
      std::vector<FileMetaInfo>& v_file_meta_info, bool is_recursive = false);
  DfsRetType ls_file(const int64_t app_id, const int64_t uid,
      const char* file_path,
      FileMetaInfo& file_meta_info);

  bool is_dir_exist(const int64_t app_id, const int64_t uid, const char* dir_path);
  bool is_file_exist(const int64_t app_id, const int64_t uid, const char* file_path);

  int32_t get_cluster_id(const int64_t app_id, const int64_t uid, const char* path);

  int64_t read(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* file_path, void* buffer, const int64_t offset, const int64_t length);
  int64_t write(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* file_path, const void* buffer, const int64_t length);
  int64_t write(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* file_path, const void* buffer, const int64_t offset, const int64_t length);

  int64_t save_file(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* local_file, const char* dfs_name);
  int64_t fetch_file(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* local_file, const char* dfs_name);

 private:
  DISALLOW_COPY_AND_ASSIGN(NameMetaClient);
  NameMetaClientImpl* impl_;

};

}
}

#endif
