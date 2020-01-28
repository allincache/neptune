#include "dfs_rc_client_api.h"
#include "base/common/Define.h"
#include "dfs_rc_client_api_impl.h"

namespace neptune {
namespace dfs {

using namespace std;

RcClient::RcClient():impl_(NULL)
{
  impl_ = new RcClientImpl();
}

RcClient::~RcClient()
{
  delete impl_;
  impl_ = NULL;
}
DfsRetType RcClient::initialize(const char* str_rc_ip, const char* app_key, const char* str_app_ip,
    const int32_t cache_times, const int32_t cache_items, const char* dev_name)
{
  int32_t real_cache_times = (cache_times >= 0) ? cache_times : DEFAULT_BLOCK_CACHE_TIME;
  int32_t real_cache_items = (cache_items >= 0) ? cache_items : DEFAULT_BLOCK_CACHE_ITEMS;
  return impl_->initialize(str_rc_ip, app_key, str_app_ip, real_cache_times, real_cache_items, dev_name, NULL);
}
DfsRetType RcClient::initialize(const uint64_t rc_ip, const char* app_key, const uint64_t app_ip,
    const int32_t cache_times, const int32_t cache_items, const char* dev_name)
{
  int32_t real_cache_times = (cache_times >= 0) ? cache_times : DEFAULT_BLOCK_CACHE_TIME;
  int32_t real_cache_items = (cache_items >= 0) ? cache_items : DEFAULT_BLOCK_CACHE_ITEMS;
  return impl_->initialize(rc_ip, app_key, app_ip, real_cache_times, real_cache_items, dev_name, NULL);
}

int64_t RcClient::get_app_id() const
{
  return impl_->get_app_id();
}

#ifdef WITH_TAIR_CACHE
void RcClient::set_remote_cache_info(const char * remote_cache_info)
{
  impl_->set_remote_cache_info(remote_cache_info);
}
#endif

void RcClient::set_client_retry_count(const int64_t count)
{
  impl_->set_client_retry_count(count);
}

int64_t RcClient::get_client_retry_count() const
{
  return impl_->get_client_retry_count();
}

void RcClient::set_client_retry_flag(bool retry_flag)
{
  impl_->set_client_retry_flag(retry_flag);
}

void RcClient::set_wait_timeout(const int64_t timeout_ms)
{
  impl_->set_wait_timeout(timeout_ms);
}

void RcClient::set_log_level(const char* level)
{
  impl_->set_log_level(level);
}

void RcClient::set_log_file(const char* log_file)
{
  impl_->set_log_file(log_file);
}

DfsRetType RcClient::logout()
{
  return impl_->logout();
}

// for raw dfs
int RcClient::open(const char* file_name, const char* suffix, const RC_MODE mode,
      const bool large, const char* local_key)
{
  return impl_->open(file_name, suffix, mode, large, local_key);
}

DfsRetType RcClient::close(const int fd, char* dfs_name_buff, const int32_t buff_len)
{
  return impl_->close(fd, dfs_name_buff, buff_len);
}

int64_t RcClient::read(const int fd, void* buf, const int64_t count)
{
  return impl_->read(fd, buf, count);
}

int64_t RcClient::readv2(const int fd, void* buf, const int64_t count, DfsFileStat* dfs_stat_buf)
{
  return impl_->readv2(fd, buf, count, dfs_stat_buf);
}

int64_t RcClient::write(const int fd, const void* buf, const int64_t count)
{
  return impl_->write(fd, buf, count);
}

int64_t RcClient::lseek(const int fd, const int64_t offset, const int whence)
{
  return impl_->lseek(fd, offset, whence);
}

DfsRetType RcClient::fstat(const int fd, DfsFileStat* buf)
{
  return impl_->fstat(fd, buf);
}

DfsRetType RcClient::unlink(const char* file_name, const char* suffix, const DfsUnlinkType action)
{
  return impl_->unlink(file_name, suffix, action);
}

int64_t RcClient::save_file(const char* local_file, char* dfs_name_buff, const int32_t buff_len,
    const char *suffix, const bool is_large_file)
{
  return impl_->save_file(local_file, dfs_name_buff, buff_len, suffix, is_large_file);
}

int64_t RcClient::save_buf(const char* source_data, const int32_t data_len,
    char* dfs_name_buff, const int32_t buff_len, const char* suffix)
{
  return impl_->save_buf(source_data, data_len, dfs_name_buff, buff_len, suffix);
}

int RcClient::fetch_file(const char* local_file,
    const char* file_name, const char* suffix)
{
  return impl_->fetch_file(local_file, file_name, suffix);
}

int RcClient::fetch_buf(int64_t& ret_count, char* buf, const int64_t count,
    const char* file_name, const char* suffix)
{
  return impl_->fetch_buf(ret_count, buf, count, file_name, suffix);
}

// for name meta
DfsRetType RcClient::create_dir(const int64_t uid, const char* dir_path)
{
  return impl_->create_dir(uid, dir_path);
}

DfsRetType RcClient::create_dir_with_parents(const int64_t uid, const char* dir_path)
{
  return impl_->create_dir_with_parents(uid, dir_path);
}

DfsRetType RcClient::create_file(const int64_t uid, const char* file_path)
{
  return impl_->create_file(uid, file_path);
}

DfsRetType RcClient::rm_dir(const int64_t uid, const char* dir_path)
{
  return impl_->rm_dir(uid, dir_path);
}

DfsRetType RcClient::rm_file(const int64_t uid, const char* file_path)
{
  return impl_->rm_file(uid, file_path);
}

DfsRetType RcClient::mv_dir(const int64_t uid, const char* src_dir_path, const char* dest_dir_path)
{
  return impl_->mv_dir(uid, src_dir_path, dest_dir_path);
}
DfsRetType RcClient::mv_file(const int64_t uid, const char* src_file_path,
    const char* dest_file_path)
{
  return impl_->mv_file(uid, src_file_path, dest_file_path);
}

DfsRetType RcClient::ls_dir(const int64_t app_id, const int64_t uid, const char* dir_path,
    std::vector<FileMetaInfo>& v_file_meta_info)
{
  return impl_->ls_dir(app_id, uid, dir_path, v_file_meta_info);
}

DfsRetType RcClient::ls_file(const int64_t app_id, const int64_t uid,
    const char* file_path,
    FileMetaInfo& file_meta_info)
{
  return impl_->ls_file(app_id, uid, file_path, file_meta_info);
}

bool RcClient::is_dir_exist(const int64_t app_id, const int64_t uid,
      const char* dir_path)
{
  return impl_->is_dir_exist(app_id, uid, dir_path);
}

bool RcClient::is_file_exist(const int64_t app_id, const int64_t uid,
      const char* file_path)
{
  return impl_->is_file_exist(app_id, uid, file_path);
}

int RcClient::open(const int64_t app_id, const int64_t uid,
    const char* name, const RcClient::RC_MODE mode)
{
  return impl_->open(app_id, uid, name, mode);
}

int64_t RcClient::pread(const int fd, void* buf, const int64_t count, const int64_t offset)
{
  return impl_->pread(fd, buf, count, offset);
}

int64_t RcClient::pwrite(const int fd, const void* buf, const int64_t count, const int64_t offset)
{
  return impl_->pwrite(fd, buf, count, offset);
}

int64_t RcClient::save_file(const int64_t uid,
    const char* local_file, const char* dfs_file_name)
{
  return impl_->save_file(get_app_id(), uid, local_file, dfs_file_name);
}

int64_t RcClient::fetch_file(const int64_t app_id, const int64_t uid,
    const char* local_file, const char* dfs_file_name)
{
  return impl_->fetch_file(app_id, uid, local_file, dfs_file_name);
}

int64_t RcClient::save_buf(const int64_t uid,
    const char* buf, const int32_t buf_len, const char* dfs_file_name)
{
  return impl_->save_buf(get_app_id(), uid, buf, buf_len, dfs_file_name);
}

int64_t RcClient::fetch_buf(const int64_t app_id, const int64_t uid,
      char* buffer, const int64_t offset, const int64_t length, const char* dfs_file_name)
{
  return impl_->fetch_buf(app_id, uid, buffer, offset, length, dfs_file_name);
}

}
}
