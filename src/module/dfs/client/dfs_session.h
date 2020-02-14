#ifndef N_DFS_CLIENT_DFSSESSION_H_
#define N_DFS_CLIENT_DFSSESSION_H_

#include "base/concurrent/Mutex.h"

#include "base/common/Internal.h"
#include "lru.h"
#include "local_key.h"

#ifdef WITH_TAIR_CACHE
#include "tair_cache_helper.h"
#endif

#ifdef WITH_UNIQUE_STORE
#include "dfs_unique_store.h"
#endif

namespace neptune {
namespace dfs {

struct BlockCache
{
  time_t last_time_;
  VUINT64 ds_;
};

class DfsSession
{
#ifdef DFS_TEST
 public:
#endif
  typedef lru<uint32_t, BlockCache> BLOCK_CACHE_MAP;
  typedef BLOCK_CACHE_MAP::iterator BLOCK_CACHE_MAP_ITER;
 
 public:
  DfsSession(const std::string& nsip, const int64_t cache_time, const int64_t cache_items);
  virtual ~DfsSession();

  int initialize();
  static void destroy();
  int get_block_info(SegmentData& seg_data, int32_t flag);
  int get_block_info(SEG_DATA_LIST& seg_list, const int32_t flag);

  void insert_local_block_cache(const uint32_t block_id, const VUINT64& rds);
  void remove_local_block_cache(const uint32_t block_id);
  bool is_hit_local_cache(const uint32_t block_id);

  inline int32_t get_cluster_id() const
  {
    return cluster_id_;
  }

  inline const std::string& get_ns_addr_str() const
  {
    return ns_addr_str_;
  }

  inline const uint64_t get_ns_addr() const
  {
    return ns_addr_;
  }

  inline const int64_t get_cache_time() const
  {
    return block_cache_time_;
  }

  inline const int64_t get_cache_items() const
  {
    return block_cache_items_;
  }

#ifdef WITH_UNIQUE_STORE
 private:
  DfsUniqueStore* unique_store_;
 public:
  int init_unique_store(const char* master_addr, const char* slave_addr,
                        const char* group_name, const int32_t area);
  inline DfsUniqueStore* get_unique_store() const
  {
    return unique_store_;
  }
#endif

#ifdef DFS_TEST
  BLOCK_CACHE_MAP* get_block_cache_map()
  {
    return &block_cache_map_;
  }
#ifdef WITH_TAIR_CACHE
  TairCacheHelper* get_remote_cache_helper()
  {
    return remote_cache_helper_;
  }
#endif
#endif

 private:
  DfsSession();
  DISALLOW_COPY_AND_ASSIGN(DfsSession);
  int get_block_info_ex(SEG_DATA_LIST& seg_list, const int32_t flag);
  int get_block_info_ex(uint32_t& block_id, VUINT64& rds, const int32_t flag);
  int get_cluster_id_from_ns();
  
 public:
  int get_cluster_group_count_from_ns();
  int get_cluster_group_seq_from_ns();

 private:
  Mutex mutex_;
#ifdef DFS_TEST
public:
  std::map<uint32_t, VUINT64> block_ds_map_; // fake meta info on ns
#endif
  uint64_t ns_addr_;
  std::string ns_addr_str_;
  int32_t cluster_id_;

#ifdef WITH_TAIR_CACHE
private:
  static TairCacheHelper* remote_cache_helper_;
public:
  int init_remote_cache_helper();
  bool is_hit_remote_cache(uint32_t block_id);
  bool check_init();
  void insert_remote_block_cache(const uint32_t block_id, const VUINT64& rds);
  int query_remote_block_cache(const uint32_t block_id, VUINT64& rds);
  int query_remote_block_cache(const SEG_DATA_LIST& seg_list, int& remote_hit_count);
  void remove_remote_block_cache(const uint32_t block_id);
#endif

 private:
  const int64_t block_cache_time_;
  const int64_t block_cache_items_;
  BLOCK_CACHE_MAP block_cache_map_;
};

}
}
#endif
