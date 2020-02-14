#include "dfs_client_api.h"
#include "dfs_client_impl.h"
#include "client_config.h"

using namespace neptune::dfs;
using namespace std;

DfsClient::DfsClient()
{
}

DfsClient::~DfsClient()
{
}

int DfsClient::initialize(const char* ns_addr, const int32_t cache_time, const int32_t cache_items,
                          const bool start_bg)
{
  return DfsClientImpl::Instance()->initialize(ns_addr, cache_time, cache_items, start_bg);
}

int DfsClient::set_default_server(const char* ns_addr, const int32_t cache_time, const int32_t cache_items)
{
  return DfsClientImpl::Instance()->set_default_server(ns_addr, cache_time, cache_items);
}

int DfsClient::destroy()
{
  return DfsClientImpl::Instance()->destroy();
}

int64_t DfsClient::read(const int fd, void* buf, const int64_t count)
{
  return DfsClientImpl::Instance()->read(fd, buf, count);
}

int64_t DfsClient::readv2(const int fd, void* buf, const int64_t count, DfsFileStat* file_info)
{
  return DfsClientImpl::Instance()->readv2(fd, buf, count, file_info);
}

int64_t DfsClient::write(const int fd, const void* buf, const int64_t count)
{
  return DfsClientImpl::Instance()->write(fd, buf, count);
}

int64_t DfsClient::lseek(const int fd, const int64_t offset, const int whence)
{
  return DfsClientImpl::Instance()->lseek(fd, offset, whence);
}

int64_t DfsClient::pread(const int fd, void* buf, const int64_t count, const int64_t offset)
{
  return DfsClientImpl::Instance()->pread(fd, buf, count, offset);
}

int64_t DfsClient::pwrite(const int fd, const void* buf, const int64_t count, const int64_t offset)
{
  return DfsClientImpl::Instance()->pwrite(fd, buf, count, offset);
}

int DfsClient::fstat(const int fd, DfsFileStat* buf, const DfsStatType mode)
{
  return DfsClientImpl::Instance()->fstat(fd, buf, mode);
}

int DfsClient::close(const int fd, char* ret_dfs_name, const int32_t ret_dfs_name_len)
{
  return DfsClientImpl::Instance()->close(fd, ret_dfs_name, ret_dfs_name_len);
}

int64_t DfsClient::get_file_length(const int fd)
{
  return DfsClientImpl::Instance()->get_file_length(fd);
}

int DfsClient::set_option_flag(const int fd, const OptionFlag option_flag)
{
  return DfsClientImpl::Instance()->set_option_flag(fd, option_flag);
}

int DfsClient:: unlink(int64_t& file_size, const char* file_name, const char* suffix,
                       const DfsUnlinkType action, const OptionFlag option_flag)
{
  return DfsClientImpl::Instance()->unlink(file_size, file_name, suffix, action, option_flag);
}

int DfsClient::unlink(int64_t& file_size, const char* file_name, const char* suffix, const char* ns_addr,
                      const DfsUnlinkType action, const OptionFlag option_flag)
{
  return DfsClientImpl::Instance()->unlink(file_size, file_name, suffix, ns_addr, action, option_flag);
}

#ifdef WITH_UNIQUE_STORE
int DfsClient::init_unique_store(const char* master_addr, const char* slave_addr,
                                 const char* group_name, const int32_t area, const char* ns_addr)
{
  return DfsClientImpl::Instance()->init_unique_store(master_addr, slave_addr, group_name, area, ns_addr);
}

int64_t DfsClient::save_buf_unique(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                               const char* buf, const int64_t count,
                               const char* suffix, const char* ns_addr)
{
  return DfsClientImpl::Instance()->save_buf_unique(ret_dfs_name, ret_dfs_name_len,
                                                buf, count, suffix, ns_addr);
}

int64_t DfsClient::save_file_unique(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                               const char* local_file,
                               const char* suffix, const char* ns_addr)
{
  return DfsClientImpl::Instance()->save_file_unique(ret_dfs_name, ret_dfs_name_len,
                                                local_file, suffix, ns_addr);
}

int32_t DfsClient::unlink_unique(int64_t& file_size, const char* file_name, const char* suffix,
                                 const char* ns_addr, const int32_t count)
{
  return DfsClientImpl::Instance()->unlink_unique(file_size, file_name, suffix, ns_addr, count);
}
#endif

void DfsClient::set_use_local_cache(const bool enable)
{
  return DfsClientImpl::Instance()->set_use_local_cache(enable);
}

void DfsClient::set_use_remote_cache(const bool enable)
{
  return DfsClientImpl::Instance()->set_use_remote_cache(enable);
}

void DfsClient::set_cache_items(const int64_t cache_items)
{
  return DfsClientImpl::Instance()->set_cache_items(cache_items);
}

int64_t DfsClient::get_cache_items() const
{
  return DfsClientImpl::Instance()->get_cache_items();
}

void DfsClient::set_cache_time(const int64_t cache_time)
{
  return DfsClientImpl::Instance()->set_cache_time(cache_time);
}

int64_t DfsClient::get_cache_time() const
{
  return DfsClientImpl::Instance()->get_cache_time();
}

#ifdef WITH_TAIR_CACHE
void DfsClient::set_remote_cache_info(const char * remote_cache_master_addr,
       const char* remote_cache_slave_addr, const char* remote_cache_group_name,
       const int32_t remote_cache_area)
{
  return DfsClientImpl::Instance()->set_remote_cache_info(remote_cache_master_addr,
           remote_cache_slave_addr, remote_cache_group_name, remote_cache_area);
}
#endif

void DfsClient::set_segment_size(const int64_t segment_size)
{
  return DfsClientImpl::Instance()->set_segment_size(segment_size);
}

int64_t DfsClient::get_segment_size() const
{
  return DfsClientImpl::Instance()->get_segment_size();
}

void DfsClient::set_batch_count(const int64_t batch_count)
{
  return DfsClientImpl::Instance()->set_batch_count(batch_count);
}

int64_t DfsClient::get_batch_count() const
{
  return DfsClientImpl::Instance()->get_batch_count();
}

void DfsClient::set_stat_interval(const int64_t stat_interval_ms)
{
  return DfsClientImpl::Instance()->set_stat_interval(stat_interval_ms);
}

int64_t DfsClient::get_stat_interval() const
{
  return DfsClientImpl::Instance()->get_stat_interval();
}

void DfsClient::set_gc_interval(const int64_t gc_interval_ms)
{
  return DfsClientImpl::Instance()->set_gc_interval(gc_interval_ms);
}

int64_t DfsClient::get_gc_interval() const
{
  return DfsClientImpl::Instance()->get_gc_interval();
}

void DfsClient::set_gc_expired_time(const int64_t gc_expired_time_ms)
{
  return DfsClientImpl::Instance()->set_gc_expired_time(gc_expired_time_ms);
}

int64_t DfsClient::get_gc_expired_time() const
{
  return DfsClientImpl::Instance()->get_gc_expired_time();
}

void DfsClient::set_batch_timeout(const int64_t timeout_ms)
{
  return DfsClientImpl::Instance()->set_batch_timeout(timeout_ms);
}

int64_t DfsClient::get_batch_timeout() const
{
  return DfsClientImpl::Instance()->get_batch_timeout();
}

void DfsClient::set_wait_timeout(const int64_t timeout_ms)
{
  return DfsClientImpl::Instance()->set_wait_timeout(timeout_ms);
}

int64_t DfsClient::get_wait_timeout() const
{
  return DfsClientImpl::Instance()->get_wait_timeout();
}

void DfsClient::set_client_retry_count(const int64_t count)
{
  return DfsClientImpl::Instance()->set_client_retry_count(count);
}

int64_t DfsClient::get_client_retry_count() const
{
  return DfsClientImpl::Instance()->get_client_retry_count();
}

void DfsClient::set_client_retry_flag(bool retry_flag)
{
  return DfsClientImpl::Instance()->set_client_retry_flag(retry_flag);
}

void DfsClient::set_log_level(const char* level)
{
  return DfsClientImpl::Instance()->set_log_level(level);
}

void DfsClient::set_log_file(const char* file)
{
  return DfsClientImpl::Instance()->set_log_file(file);
}

int32_t DfsClient::get_block_cache_time() const
{
  return DfsClientImpl::Instance()->get_block_cache_time();
}

int32_t DfsClient::get_block_cache_items() const
{
  return DfsClientImpl::Instance()->get_block_cache_items();
}

int32_t DfsClient::get_cache_hit_ratio(CacheType cache_type) const
{
  return DfsClientImpl::Instance()->get_cache_hit_ratio(cache_type);
}

uint64_t DfsClient::get_server_id()
{
  return DfsClientImpl::Instance()->get_server_id();
}

int32_t DfsClient::get_cluster_id(const char* ns_addr)
{
  return DfsClientImpl::Instance()->get_cluster_id(ns_addr);
}

int64_t DfsClient::save_buf(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                             const char* buf, const int64_t count,
                             const int32_t flag, const char* suffix, const char* ns_addr, const char* key)
{
  return DfsClientImpl::Instance()->save_buf(ret_dfs_name, ret_dfs_name_len,
                                              buf, count, flag, suffix, ns_addr, key);
}

int64_t DfsClient::save_file(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                             const char* local_file, const int32_t flag, const char* suffix, const char* ns_addr)
{
  return DfsClientImpl::Instance()->save_file(ret_dfs_name, ret_dfs_name_len,
                                              local_file, flag, suffix, ns_addr);
}

int DfsClient::fetch_file(const char* local_file, const char* file_name,
                          const char* suffix, const char* ns_addr)
{
  return DfsClientImpl::Instance()->fetch_file(local_file, file_name, suffix, ns_addr);
}

int DfsClient::fetch_file(int64_t& ret_count, char* buf, const int64_t count, const char* file_name, const char* suffix, const char* ns_addr)
{
  return DfsClientImpl::Instance()->fetch_file(ret_count, buf, count, file_name, suffix, ns_addr);
}

int DfsClient::stat_file(DfsFileStat* file_stat, const char* file_name, const char* suffix,
                         const DfsStatType stat_type, const char* ns_addr)
{
  return DfsClientImpl::Instance()->stat_file(file_stat, file_name, suffix, stat_type, ns_addr);
}

int DfsClient::open(const char* file_name, const char* suffix, const int flags, const char* key)
{
  return open(file_name, suffix, NULL, flags, key);
}

int DfsClient::open(const char* file_name, const char* suffix, const char* ns_addr, const int flags, const char* key)
{
  int ret = EXIT_INVALIDFD_ERROR;

  if ((flags & T_LARGE) && (flags & T_WRITE) && NULL == key)
  {
    LOG(ERROR, "open with T_LARGE|T_WRITE but without key");
  }
  else
  {
    ret = DfsClientImpl::Instance()->open(file_name, suffix, ns_addr, flags, key);
  }

  return ret;
}
