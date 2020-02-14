#ifndef N_DFS_CLIENT_DFSCLIENTIMPL_H_
#define N_DFS_CLIENT_DFSCLIENTIMPL_H_

#include "base/concurrent/Mutex.h"
#include <stdio.h>
#include <pthread.h>
#include "base/common/Internal.h"
#include "dfs_session_pool.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class BasePacketFactory;
class BasePacketStreamer;

// class Mutex;
class DfsFile;
class DfsSession;
class GcWorker;
typedef std::map<int, DfsFile*> FILE_MAP;

class DfsClientImpl
{
 public:
  static DfsClientImpl* Instance()
  {
    static DfsClientImpl dfs_client_impl;
    return &dfs_client_impl;
  }

  int initialize(const char* ns_addr, const int32_t cache_time, const int32_t cache_items, const bool start_bg);
  int set_default_server(const char* ns_addr, const int32_t cache_time, const int32_t cache_items);
  int destroy();

  int open(const char* file_name, const char* suffix, const char* ns_addr, const int flags, ...);
  int64_t read(const int fd, void* buf, const int64_t count);
  int64_t readv2(const int fd, void* buf, const int64_t count, DfsFileStat* file_info);
  int64_t write(const int fd, const void* buf, const int64_t count);
  int64_t lseek(const int fd, const int64_t offset, const int whence);
  int64_t pread(const int fd, void* buf, const int64_t count, const int64_t offset);
  int64_t pwrite(const int fd, const void* buf, const int64_t count, const int64_t offset);
  int fstat(const int fd, DfsFileStat* buf, const DfsStatType mode = NORMAL_STAT);
  int close(const int fd, char* ret_dfs_name = NULL, const int32_t ret_dfs_name_len = 0, const bool simple = false);
  int64_t get_file_length(const int fd);

  int set_option_flag(const int fd, const OptionFlag option_flag);

  int unlink(int64_t & size, const char* file_name, const char* suffix ,
              const DfsUnlinkType action = DELETE,
              const OptionFlag option_flag = DFS_FILE_DEFAULT_OPTION);

  int unlink(int64_t& file_size, const char* file_name, const char* suffix, const char* ns_addr,
              const DfsUnlinkType action = DELETE,
              const OptionFlag option_flag = DFS_FILE_DEFAULT_OPTION);

  void set_cache_items(const int64_t cache_items);
  int64_t get_cache_items() const;

  void set_cache_time(const int64_t cache_time);
  int64_t get_cache_time() const;

  void set_use_local_cache(const bool enable = true);
  void set_use_remote_cache(const bool enable = true);
  // for test
  void insert_local_block_cache(const char* ns_addr, const uint32_t block_id, const VUINT64& ds_list);
  void remove_local_block_cache(const char* ns_addr, const uint32_t block_id);
  bool is_hit_local_cache(const char* ns_addr, const char* dfs_name) const;
#ifdef WITH_TAIR_CACHE
  void set_remote_cache_info(const char* remote_cache_master_addr, const char* remote_cache_slave_addr,
          const char* remote_cache_group_name, const int32_t area);
  void insert_remote_block_cache(const char* ns_addr, const uint32_t block_id, const VUINT64& ds_list);
  void remove_remote_block_cache(const char* ns_addr, const uint32_t block_id);
  bool is_hit_remote_cache(const char* ns_addr, const char* dfs_name) const;
#endif

  void set_segment_size(const int64_t segment_size);
  int64_t get_segment_size() const;

  void set_batch_count(const int64_t batch_count);
  int64_t get_batch_count() const;

  void set_stat_interval(const int64_t stat_interval_ms);
  int64_t get_stat_interval() const;

  void set_gc_interval(const int64_t gc_interval_ms);
  int64_t get_gc_interval() const;

  void set_gc_expired_time(const int64_t gc_expired_time_ms);
  int64_t get_gc_expired_time() const;

  void set_batch_timeout(const int64_t time_out_ms);
  int64_t get_batch_timeout() const;

  void set_wait_timeout(const int64_t time_out_ms);
  int64_t get_wait_timeout() const;

  void set_client_retry_count(const int64_t count);
  int64_t get_client_retry_count() const;
  void set_client_retry_flag(bool retry_flag);

  void set_log_level(const char* level);
  void set_log_file(const char* file);

  int32_t get_block_cache_time() const;
  int32_t get_block_cache_items() const;
  int32_t get_cache_hit_ratio(CacheType cache_type = LOCAL_CACHE) const;

#ifdef DFS_TEST
  DfsSession* get_dfs_session(const char* ns_addr)
    {
      return SESSION_POOL.get(ns_addr);
    }
#endif

#ifdef WITH_UNIQUE_STORE
#include "dfs_unique_store.h"
  DfsUniqueStore* get_unique_store(const char* ns_addr);
  int init_unique_store(const char* master_addr, const char* slave_addr,
                        const char* group_name, const int32_t area, const char* ns_addr = NULL);
  int64_t save_buf_unique(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                      const char* buf, const int64_t count,
                      const char* suffix = NULL, const char* ns_addr = NULL);
  int64_t save_file_unique(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                      const char* local_file,
                      const char* suffix = NULL, const char* ns_addr = NULL);
  int64_t save_buf_unique_update(const char* buf, const int64_t count,
                              const char* file_name, const char* suffix = NULL, const char* ns_addr = NULL);
  int64_t save_file_unique_update(const char* local_file,
                              const char* file_name, const char* suffix = NULL, const char* ns_addr = NULL);
  int32_t unlink_unique(int64_t& file_size, const char* file_name, const char* suffix = NULL,
                        const char* ns_addr = NULL, const int32_t count = 1);

  int64_t save_buf_unique_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                          const char* buf, const int64_t count,
                          const char* file_name, const char* suffix, const char* ns_addr);
  int64_t save_file_unique_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                          const char* local_file,
                          const char* file_name, const char* suffix, const char* ns_addr);
#endif

  // sort of utility
  uint64_t get_server_id();
  int32_t get_cluster_id(const char* ns_addr = NULL);
  int32_t get_cluster_group_count(const char* ns_addr = NULL);
  int32_t get_cluster_group_seq(const char* ns_addr = NULL);

  int64_t save_buf(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                    const char* buf, const int64_t count,
                    const int32_t flag, const char* suffix = NULL,
                    const char* ns_addr = NULL, const char* key = NULL, const bool simple = false);
  int64_t save_file(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                    const char* local_file,
                    const int32_t flag, const char* suffix = NULL,
                    const char* ns_addr = NULL, bool simple = false);
  int64_t save_file_update(const char* buf, const int64_t count,
                            const int32_t flag,
                            const char* file_name, const char* suffix = NULL,
                            const char* ns_addr = NULL, const char* key = NULL);
  int64_t save_file_update(const char* local_file,
                            const int32_t flag,
                            const char* file_name, const char* suffix = NULL,
                            const char* ns_addr = NULL);
  int fetch_file(int64_t& ret_count, char* buf, const int64_t count,
                  const char* file_name, const char* suffix = NULL, const char* ns_addr = NULL, const int flags = T_DEFAULT);
  int fetch_file(const char* local_file,
                  const char* file_name, const char* suffix = NULL, const char* ns_addr = NULL, const int flags = T_DEFAULT);

  int stat_file(DfsFileStat* file_stat, const char* file_name, const char* suffix = NULL,
                const DfsStatType stat_type = NORMAL_STAT, const char* ns_addr = NULL);

  int64_t save_file_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                        const char* local_file, const int32_t flag,
                        const char* file_name, const char* suffix = NULL,
                        const char* ns_addr = NULL, bool simple = false);
  int64_t save_buf_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                        const char* buf, const int64_t count, const int32_t flag,
                        const char* file_name, const char* suffix = NULL,
                        const char* ns_addr = NULL, const char* key = NULL, bool simple = false);
  // fetch file to buffer, return count
  // WARNING: user MUST free buf.
  int fetch_file_ex(char*& buf, int64_t& count,
                    const char* file_name, const char* suffix = NULL, const char* ns_addr = NULL);

private:
  bool check_init();
  DfsSession* get_session(const char* ns_addr);
  int get_fd();
  DfsFile* get_file(const int fd);
  int insert_file(const int fd, DfsFile* dfs_file);
  int erase_file(const int fd);
  bool is_hit_local_cache(const char* ns_addr, const uint32_t block_id) const;
#ifdef WITH_TAIR_CACHE
  bool is_hit_remote_cache(const char* ns_addr, const uint32_t block_id) const;
#endif

private:
  DfsClientImpl();
  DISALLOW_COPY_AND_ASSIGN(DfsClientImpl);
  ~DfsClientImpl();

  bool is_init_;
  DfsSession* default_dfs_session_;
  int fd_;
  FILE_MAP dfs_file_map_;
  Mutex mutex_;
  BasePacketFactory* packet_factory_;
  BasePacketStreamer* packet_streamer_;
};

}
}

#endif  // N_DFS_CLIENT_DFSCLIENTAPI_H_
