#include <stdarg.h>
#include <string>
#include "base/common/Memory.h"
#include "dfs/util/base_packet_factory.h"
#include "dfs/util/base_packet_streamer.h"
#include "dfs/util/client_manager.h"
#include "dfs/message/message_factory.h"
#include "dfs_client_impl.h"
#include "dfs_large_file.h"
#include "dfs_small_file.h"
#include "gc_worker.h"
#include "bg_task.h"
#include "base/fs/FsName.h"

using namespace neptune::dfs;
using namespace std;

DfsClientImpl::DfsClientImpl() : is_init_(false), default_dfs_session_(NULL), fd_(0),
                                 packet_factory_(NULL), packet_streamer_(NULL)
{
  packet_factory_ = new MessageFactory();
  packet_streamer_ = new BasePacketStreamer(packet_factory_);
}

DfsClientImpl::~DfsClientImpl()
{
  for (FILE_MAP::iterator it = dfs_file_map_.begin(); it != dfs_file_map_.end(); ++it)
  {
    gDelete(it->second);
  }
  dfs_file_map_.clear();

  gDelete(packet_factory_);
  gDelete(packet_streamer_);
}

int DfsClientImpl::initialize(const char* ns_addr, const int32_t cache_time, const int32_t cache_items,
                              const bool start_bg)
{
  int ret = SUCCESS;

  Mutex::Lock lock(mutex_);
  if (is_init_)
  {
    ////LOG(INFO, "dfsclient already initialized");
  }
  else if (cache_items < 0 || cache_time <= 0)
  {
    //LOG(ERROR, "invalid cache config. cache_time: %d, cache_items: %d", cache_time, cache_items);
    ret = ERROR;
  }
  else if (SUCCESS != (ret = NewClientManager::get_instance().initialize(packet_factory_, packet_streamer_)))
  {
    //LOG(ERROR, "initialize NewClientManager fail, must exit, ret: %d", ret);
  }
  else if (ns_addr != NULL && strlen(ns_addr) > 0 &&   // pass a valid ns addr, then must init success
           NULL == (default_dfs_session_ = SESSION_POOL.get(ns_addr, cache_time, cache_items)))
  {
    //LOG(ERROR, "dfsclient initialize to ns %s failed. must exit", ns_addr);
    ret = ERROR;
  }
  else if (start_bg)
  {
    if ((ret = BgTask::initialize()) != SUCCESS)
    {
      //LOG(ERROR, "start bg task fail, must exit. ret: %d", ret);
    }
  }

  if (SUCCESS == ret && !is_init_)
  {
    set_cache_time(cache_time);
    set_cache_items(cache_items);
    is_init_ = true;
  }

  return ret;
}

int DfsClientImpl::set_default_server(const char* ns_addr, const int32_t cache_time, const int32_t cache_items)
{
  int ret = ERROR;
  DfsSession* session = NULL;
  if (NULL == ns_addr)
  {
    //LOG(ERROR, "ns addr is null");
  }
  else if ((session = SESSION_POOL.get(ns_addr, cache_time, cache_items)) == NULL)
  {
    //LOG(ERROR, "get session to server %s fail.", ns_addr);
  }
  else
  {
    default_dfs_session_ = session;
    ret = SUCCESS;
  }
  return ret;
}

int DfsClientImpl::destroy()
{
  Mutex::Lock lock(mutex_);
  if (is_init_)
  {
    DfsSession::destroy();
    BgTask::destroy();
    BgTask::wait_for_shut_down();
    is_init_ = false;
  }
  return SUCCESS;
}

int64_t DfsClientImpl::read(const int fd, void* buf, const int64_t count)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    // modify offset_: use write locker
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
    ret = dfs_file->read(buf, count);
  }
  return ret;
}

int64_t DfsClientImpl::readv2(const int fd, void* buf, const int64_t count, DfsFileStat* file_info)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    // modify offset_: use write locker
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
    ret = dfs_file->readv2(buf, count, file_info);
  }
  return ret;
}

int64_t DfsClientImpl::write(const int fd, const void* buf, const int64_t count)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
    ret = dfs_file->write(buf, count);
  }
  return ret;
}

int64_t DfsClientImpl::lseek(const int fd, const int64_t offset, const int whence)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    // modify offset_: use write locker
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
    ret = dfs_file->lseek(offset, whence);
  }
  return ret;
}

int64_t DfsClientImpl::pread(const int fd, void* buf, const int64_t count, const int64_t offset)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, READ_LOCKER);
    ret = dfs_file->pread(buf, count, offset);
  }
  return ret;
}

int64_t DfsClientImpl::pwrite(const int fd, const void* buf, const int64_t count, const int64_t offset)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
    ret = dfs_file->pwrite(buf, count, offset);
  }
  return ret;
}

int DfsClientImpl::fstat(const int fd, DfsFileStat* buf, const DfsStatType mode)
{
  int ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
    ret = dfs_file->fstat(buf, mode);
  }
  return ret;
}

int DfsClientImpl::close(const int fd, char* ret_dfs_name, const int32_t ret_dfs_name_len, const bool simple)
{
  int ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    {
      ScopedRWLock scoped_lock(dfs_file->rw_lock_, WRITE_LOCKER);
      ret = dfs_file->close();
      if (SUCCESS != ret)
      {
        //LOG(ERROR, "dfs close failed. fd: %d, ret: %d", fd, ret);
      }
      // buffer not null, then consider as wanting dfs name back
      // len must invalid
      else if (NULL != ret_dfs_name)
      {
        if (ret_dfs_name_len < DFS_FILE_LEN)
        {
          //LOG(ERROR, "name buffer length less: %d < %d", ret_dfs_name_len, DFS_FILE_LEN);
          ret = ERROR;
        }
        else
        {
          memcpy(ret_dfs_name, dfs_file->get_file_name(simple), DFS_FILE_LEN);
        }
      }
    }
    erase_file(fd);
  }

  return ret;
}

int64_t DfsClientImpl::get_file_length(const int fd)
{
  int64_t ret = EXIT_INVALIDFD_ERROR;
  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    ScopedRWLock scoped_lock(dfs_file->rw_lock_, READ_LOCKER);
    ret = dfs_file->get_file_length();
  }
  return ret;
}

int DfsClientImpl::open(const char* file_name, const char* suffix, const char* ns_addr, const int flags, ...)
{
  int ret_fd = EXIT_INVALIDFD_ERROR;
  DfsSession* dfs_session = NULL;

  if (!check_init())
  {
    //LOG(ERROR, "dfs client not init");
  }
  else if (NULL == (dfs_session = get_session(ns_addr)))
  {
    //LOG(ERROR, "can not get dfs session: %s.", NULL == ns_addr ? "default" : ns_addr);
  }
  else if ((ret_fd = get_fd()) <= 0)
  {
    //LOG(ERROR, "can not get fd. ret: %d", ret_fd);
  }
  else
  {
    DfsFile* dfs_file = NULL;
    int ret = ERROR;

    if (0 == (flags & T_LARGE))
    {
      dfs_file = new DfsSmallFile();
      dfs_file->set_session(dfs_session);
      ret = dfs_file->open(file_name, suffix, flags);
    }
    else
    {
      va_list args;
      va_start(args, flags);
      dfs_file = new DfsLargeFile();
      dfs_file->set_session(dfs_session);
      ret = dfs_file->open(file_name, suffix, flags, va_arg(args, char*));
      va_end(args);
    }

    if (ret != SUCCESS)
    {
      //LOG(ERROR, "open DfsFile fail, filename: %s, suffix: %s, flags: %d, ret: %d", file_name, suffix, flags, ret);
    }
    else if ((ret = insert_file(ret_fd, dfs_file)) != SUCCESS)
    {
      //LOG(ERROR, "add fd fail: %d", ret_fd);
    }

    if (ret != SUCCESS)
    {
      ret_fd = (ret < 0) ? ret : EXIT_INVALIDFD_ERROR; // return true error code except ERROR
      gDelete(dfs_file);
    }
  }

  return ret_fd;
}

int DfsClientImpl::set_option_flag(const int fd, const OptionFlag option_flag)
{
  int ret = EXIT_INVALIDFD_ERROR;

  DfsFile* dfs_file = get_file(fd);
  if (NULL != dfs_file)
  {
    dfs_file->set_option_flag(option_flag);
    ret = SUCCESS;
  }
  return ret;
}

int DfsClientImpl::unlink(int64_t& file_size, const char* file_name, const char* suffix,
                          const DfsUnlinkType action, const OptionFlag option_flag)
{
  return unlink(file_size, file_name, suffix, NULL, action, option_flag);
}

int DfsClientImpl::unlink(int64_t& file_size, const char* file_name, const char* suffix, const char* ns_addr,
                          const DfsUnlinkType action, const OptionFlag option_flag)
{
  int ret = ERROR;
  DfsSession* dfs_session = NULL;

  if (!check_init())
  {
    //LOG(ERROR, "dfs client not init");
  }
  else if (NULL == (dfs_session = get_session(ns_addr)))
  {
    //LOG(ERROR, "can not get dfs session: %s.", NULL == ns_addr ? "default" : ns_addr);
  }
  else
  {
    DfsFile* dfs_file = NULL;
    BaseFileType file_type = FSName::check_file_type(file_name);
    if (file_type == SMALL_DFS_FILE_TYPE)
    {
      dfs_file = new DfsSmallFile();
      dfs_file->set_session(dfs_session);
      dfs_file->set_option_flag(option_flag);
      ret = dfs_file->unlink(file_name, suffix, file_size, action);
    }
    else if (file_type == LARGE_DFS_FILE_TYPE)
    {
      dfs_file = new DfsLargeFile();
      dfs_file->set_session(dfs_session);
      dfs_file->set_option_flag(option_flag);
      ret = dfs_file->unlink(file_name, suffix, file_size, action);
    }
    else
    {
      //LOG(ERROR, "dfs file name illegal: %s", file_name);
    }

    gDelete(dfs_file);
  }
  return ret;
}

#ifdef WITH_UNIQUE_STORE
DfsUniqueStore* DfsClientImpl::get_unique_store(const char* ns_addr)
{
  DfsUniqueStore* unique_store = NULL;
  DfsSession* session = get_session(ns_addr);

  if (session != NULL)
  {
    unique_store = session->get_unique_store();
  }
  else
  {
    //LOG(ERROR, "session not init");
  }

  return unique_store;
}

int DfsClientImpl::init_unique_store(const char* master_addr, const char* slave_addr,
                                     const char* group_name, const int32_t area, const char* ns_addr)
{
  int ret = ERROR;
  DfsSession* session = get_session(ns_addr);

  if (NULL == session)
  {
    //LOG(ERROR, "session not init");
  }
  else
  {
    ret = session->init_unique_store(master_addr, slave_addr, group_name, area);
  }

  return ret;
}

int64_t DfsClientImpl::save_buf_unique(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                   const char* buf, const int64_t count,
                                   const char* suffix, const char* ns_addr)
{
  int64_t ret = INVALID_FILE_SIZE;
  if (NULL == ret_dfs_name || ret_dfs_name_len < DFS_FILE_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_buf_unique_ex(ret_dfs_name, ret_dfs_name_len, buf, count, NULL, suffix, ns_addr);
  }

  return ret;
}

int64_t DfsClientImpl::save_file_unique(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                   const char* local_file,
                                   const char* suffix, const char* ns_addr)
{
  int64_t ret =  INVALID_FILE_SIZE;

  if (NULL == ret_dfs_name || ret_dfs_name_len < DFS_FILE_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_file_unique_ex(ret_dfs_name, ret_dfs_name_len, local_file, NULL, suffix, ns_addr);
  }

  return ret;
}

int64_t DfsClientImpl::save_buf_unique_update(const char* buf, const int64_t count,
                                          const char* file_name, const char* suffix, const char* ns_addr)
{
  int64_t ret =  INVALID_FILE_SIZE;

  if (NULL == file_name || static_cast<int32_t>(strlen(file_name)) < FILE_NAME_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_buf_unique_ex(NULL, 0, buf, count, file_name, suffix, ns_addr);
  }

  return ret;
}

int64_t DfsClientImpl::save_file_unique_update(const char* local_file,
                                          const char* file_name, const char* suffix, const char* ns_addr)
{
  int64_t ret =  INVALID_FILE_SIZE;

  if (NULL == file_name || static_cast<int32_t>(strlen(file_name)) < FILE_NAME_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_file_unique_ex(NULL, 0, local_file, file_name, suffix, ns_addr);
  }

  return ret;
}

int32_t DfsClientImpl::unlink_unique(int64_t& file_size, const char* file_name, const char* suffix,
                                     const char* ns_addr, const int32_t count)
{
  int32_t ret = INVALID_REFERENCE_COUNT;
  DfsUniqueStore* unique_store = get_unique_store(ns_addr);

  if (unique_store != NULL)
  {
    ret = unique_store->unlink(file_name, suffix, file_size, count);
  }
  else
  {
    //LOG(ERROR, "unique store not init");
  }

  return ret;
}

int64_t DfsClientImpl::save_buf_unique_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                      const char* buf, const int64_t count,
                                      const char* file_name, const char* suffix, const char* ns_addr)
{
  int64_t ret = INVALID_FILE_SIZE;
  if (NULL == buf || count <= 0)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    DfsUniqueStore* unique_store = get_unique_store(ns_addr);
    if (unique_store != NULL)
    {
      ret = unique_store->save(buf, count, file_name, suffix, ret_dfs_name, ret_dfs_name_len);
    }
    else
    {
      //LOG(ERROR, "unique store not init");
    }
  }

  return ret;
}

int64_t DfsClientImpl::save_file_unique_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                      const char* local_file,
                                      const char* file_name, const char* suffix, const char* ns_addr)
{
  int64_t ret = INVALID_FILE_SIZE;
  if (NULL == local_file)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    DfsUniqueStore* unique_store = get_unique_store(ns_addr);
    if (unique_store != NULL)
    {
      ret = unique_store->save(local_file, file_name, suffix, ret_dfs_name, ret_dfs_name_len);
    }
    else
    {
      //LOG(ERROR, "unique store not init");
    }
  }
  return ret;
}
#endif

void DfsClientImpl::set_cache_items(const int64_t cache_items)
{
  if (cache_items >= 0)
  {
    ClientConfig::cache_items_ = cache_items;
    //LOG(INFO, "set cache items: %"PRI64_PREFIX"d", ClientConfig::cache_items_);
    ClientConfig::use_cache_ |= USE_CACHE_FLAG_LOCAL;
  }
  else
  {
    //LOG(WARN, "set cache items invalid: %"PRI64_PREFIX"d", cache_items);
  }
}

int64_t DfsClientImpl::get_cache_items() const
{
  return ClientConfig::cache_items_;
}

void DfsClientImpl::set_cache_time(const int64_t cache_time)
{
  if (cache_time > 0)
  {
    ClientConfig::cache_time_ = cache_time;
    //LOG(INFO, "set cache time: %"PRI64_PREFIX"d", ClientConfig::cache_time_);
    ClientConfig::use_cache_ |= USE_CACHE_FLAG_LOCAL;
  }
  else
  {
    //LOG(WARN, "set cache time invalid: %"PRI64_PREFIX"d", cache_time);
  }
}

int64_t DfsClientImpl::get_cache_time() const
{
  return ClientConfig::cache_time_;
}

void DfsClientImpl::set_use_local_cache(const bool enable)
{
  if (enable)
  {
    ClientConfig::use_cache_ |= USE_CACHE_FLAG_LOCAL;
  }
  else
  {
    ClientConfig::use_cache_ &= ~USE_CACHE_FLAG_LOCAL;
  }
}

void DfsClientImpl::set_use_remote_cache(const bool enable)
{
  if (enable)
  {
    ClientConfig::use_cache_ |= USE_CACHE_FLAG_REMOTE;
  }
  else
  {
    ClientConfig::use_cache_ &= ~USE_CACHE_FLAG_REMOTE;
  }
}

void DfsClientImpl::insert_local_block_cache(const char* ns_addr, const uint32_t block_id,
       const VUINT64& ds_list)
{
  DfsSession *dfs_session = NULL;
  if (ns_addr != NULL && strlen(ns_addr) > 0)
  {
    dfs_session = SESSION_POOL.get(ns_addr);
  }
  else
  {
    if (NULL != default_dfs_session_)
    {
      dfs_session = default_dfs_session_;
    }
  }
  if (NULL != dfs_session)
  {
    dfs_session->insert_local_block_cache(block_id, ds_list);
  }
}

void DfsClientImpl::remove_local_block_cache(const char* ns_addr, const uint32_t block_id)
{
  DfsSession *dfs_session = NULL;
  if (ns_addr != NULL && strlen(ns_addr) > 0)
  {
    dfs_session = SESSION_POOL.get(ns_addr);
  }
  else
  {
    if (NULL != default_dfs_session_)
    {
      dfs_session = default_dfs_session_;
    }
  }
  if (NULL != dfs_session)
  {
    dfs_session->remove_local_block_cache(block_id);
  }
}

bool DfsClientImpl::is_hit_local_cache(const char* ns_addr, const char* dfs_name) const
{
  bool ret = false;
  if (NULL == dfs_name || dfs_name[0] == '\0')
  {
    //LOG(ERROR, "dfs_name is NULL or empty");
  }
  else
  {
    FSName fs_name(dfs_name);
    uint32_t block_id = fs_name.get_block_id();
    ret = is_hit_local_cache(ns_addr, block_id);
  }
  return ret;
}

#ifdef WITH_TAIR_CACHE
void DfsClientImpl::set_remote_cache_info(const char* remote_cache_master_addr, const char* remote_cache_slave_addr,
       const char* remote_cache_group_name, const int32_t remote_cache_area)
{
  ClientConfig::remote_cache_master_addr_ = remote_cache_master_addr;
  ClientConfig::remote_cache_slave_addr_ = remote_cache_slave_addr;
  ClientConfig::remote_cache_group_name_ = remote_cache_group_name;
  ClientConfig::remote_cache_area_ = remote_cache_area;
}

void DfsClientImpl::insert_remote_block_cache(const char* ns_addr, const uint32_t block_id,
       const VUINT64& ds_list)
{
  DfsSession *dfs_session = NULL;
  if (ns_addr != NULL && strlen(ns_addr) > 0)
  {
    dfs_session = SESSION_POOL.get(ns_addr);
  }
  else
  {
    if (NULL != default_dfs_session_)
    {
      dfs_session = default_dfs_session_;
    }
  }
  if (NULL != dfs_session)
  {
    dfs_session->insert_remote_block_cache(block_id, ds_list);
  }
}

void DfsClientImpl::remove_remote_block_cache(const char* ns_addr, const uint32_t block_id)
{
  DfsSession *dfs_session = NULL;
  if (ns_addr != NULL && strlen(ns_addr) > 0)
  {
    dfs_session = SESSION_POOL.get(ns_addr);
  }
  else
  {
    if (NULL != default_dfs_session_)
    {
      dfs_session = default_dfs_session_;
    }
  }
  if (NULL != dfs_session)
  {
    dfs_session->remove_remote_block_cache(block_id);
  }
}

bool DfsClientImpl::is_hit_remote_cache(const char* ns_addr, const char* dfs_name) const
{
  bool ret = false;
  if (NULL == dfs_name || dfs_name[0] == '\0')
  {
    //LOG(ERROR, "dfs_name is NULL or empty");
  }
  else
  {
    FSName fs_name(dfs_name);
    uint32_t block_id = fs_name.get_block_id();
    ret = is_hit_remote_cache(ns_addr, block_id);
  }
  return ret;
}

bool DfsClientImpl::is_hit_remote_cache(const char* ns_addr, const uint32_t block_id) const
{
  DfsSession *dfs_session = NULL;
  bool ret = false;

  if (ns_addr != NULL && strlen(ns_addr) > 0)
  {
    dfs_session = SESSION_POOL.get(ns_addr);
  }
  else
  {
    if (NULL != default_dfs_session_)
    {
      dfs_session = default_dfs_session_;
    }
  }

  if (NULL != dfs_session)
  {
    ret = dfs_session->is_hit_remote_cache(block_id);
  }

  return ret;

}
#endif

void DfsClientImpl::set_segment_size(const int64_t segment_size)
{
  if (segment_size > 0 && segment_size <= MAX_SEGMENT_SIZE)
  {
    ClientConfig::segment_size_ = segment_size;
    ClientConfig::batch_size_ = ClientConfig::segment_size_ * ClientConfig::batch_count_;
    //LOG(INFO, "set segment size: %" PRI64_PREFIX "d, batch count: %" PRI64_PREFIX "d, batch size: %" PRI64_PREFIX "d",
    //          ClientConfig::segment_size_, ClientConfig::batch_count_, ClientConfig::batch_size_);
  }
  else
  {
    //LOG(WARN, "set segment size %"PRI64_PREFIX"d not in (0, %"PRI64_PREFIX"d]", segment_size, MAX_SEGMENT_SIZE);
  }
}

int64_t DfsClientImpl::get_segment_size() const
{
  return ClientConfig::segment_size_;
}

void DfsClientImpl::set_batch_count(const int64_t batch_count)
{
  if (batch_count > 0 && batch_count <= MAX_BATCH_COUNT)
  {
    ClientConfig::batch_count_ = batch_count;
    ClientConfig::batch_size_ = ClientConfig::segment_size_ * ClientConfig::batch_count_;
    //LOG(INFO, "set batch count: %" PRI64_PREFIX "d, segment size: %" PRI64_PREFIX "d, batch size: %" PRI64_PREFIX "d",
    //          ClientConfig::batch_count_, ClientConfig::segment_size_, ClientConfig::batch_size_);
  }
  else
  {
    //LOG(WARN, "set batch count %"PRI64_PREFIX"d not in (0, %"PRI64_PREFIX"d]", batch_count, MAX_BATCH_COUNT);
  }
}

int64_t DfsClientImpl::get_batch_count() const
{
  return ClientConfig::batch_count_;
}

void DfsClientImpl::set_stat_interval(const int64_t stat_interval_ms)
{
  if (stat_interval_ms > 0)
  {
    ClientConfig::stat_interval_ = stat_interval_ms;
    BgTask::get_stat_mgr().reset_schedule_interval(stat_interval_ms * 1000);
    //LOG(INFO, "set stat interval: %" PRI64_PREFIX "d ms", ClientConfig::stat_interval_);
  }
  else
  {
    //LOG(WARN, "set stat interval %"PRI64_PREFIX"d <= 0", stat_interval_ms);
  }
}

int64_t DfsClientImpl::get_stat_interval() const
{
  return ClientConfig::stat_interval_;
}

void DfsClientImpl::set_gc_interval(const int64_t gc_interval_ms)
{
  if (gc_interval_ms > 0)
  {
    ClientConfig::gc_interval_ = gc_interval_ms;
    BgTask::get_gc_mgr().reset_schedule_interval(gc_interval_ms);
    //LOG(INFO, "set gc interval: %" PRI64_PREFIX "d ms", ClientConfig::gc_interval_);
  }
  else
  {
    //LOG(WARN, "set gc interval %"PRI64_PREFIX"d <= 0", gc_interval_ms);
  }
}

int64_t DfsClientImpl::get_gc_interval() const
{
  return ClientConfig::gc_interval_;
}

void DfsClientImpl::set_gc_expired_time(const int64_t gc_expired_time_ms)
{
  if (gc_expired_time_ms >= MIN_GC_EXPIRED_TIME)
  {
    ClientConfig::expired_time_ = gc_expired_time_ms;
    //LOG(INFO, "set gc expired time: %" PRI64_PREFIX "d ms", ClientConfig::expired_time_);
  }
  else
  {
    //LOG(WARN, "set gc expired interval %"PRI64_PREFIX"d < %"PRI64_PREFIX"d",
    //          gc_expired_time_ms, MIN_GC_EXPIRED_TIME);
  }
}

int64_t DfsClientImpl::get_gc_expired_time() const
{
  return ClientConfig::expired_time_;
}

void DfsClientImpl::set_batch_timeout(const int64_t timeout_ms)
{
  if (timeout_ms > 0)
  {
    ClientConfig::batch_timeout_ = timeout_ms;
    //LOG(INFO, "set batch timeout: %" PRI64_PREFIX "d ms", ClientConfig::batch_timeout_);
  }
  else
  {
    //LOG(WARN, "set batch timeout %"PRI64_PREFIX"d <= 0", timeout_ms);
  }
}

int64_t DfsClientImpl::get_batch_timeout() const
{
  return ClientConfig::batch_timeout_;
}

void DfsClientImpl::set_wait_timeout(const int64_t timeout_ms)
{
  if (timeout_ms > 0)
  {
    ClientConfig::wait_timeout_ = timeout_ms;
    //LOG(INFO, "set wait timeout: %" PRI64_PREFIX "d ms", ClientConfig::wait_timeout_);
  }
  else
  {
    //LOG(WARN, "set wait timeout %"PRI64_PREFIX"d <= 0", timeout_ms);
  }
}

int64_t DfsClientImpl::get_wait_timeout() const
{
  return ClientConfig::wait_timeout_;
}

void DfsClientImpl::set_client_retry_count(const int64_t count)
{
  if (count > 0)
  {
    ClientConfig::client_retry_count_ = count;
    //LOG(INFO, "set client retry count: %" PRI64_PREFIX "d", ClientConfig::client_retry_count_);
  }
  else
  {
    //LOG(WARN, "set client retry count %"PRI64_PREFIX"d <= 0", count);
  }
}

int64_t DfsClientImpl::get_client_retry_count() const
{
  return ClientConfig::client_retry_count_;
}

void DfsClientImpl::set_client_retry_flag(bool retry_flag)
{
  ClientConfig::client_retry_flag_ = retry_flag;
  //LOG(INFO, "set client retry flag: %d", ClientConfig::client_retry_flag_);
}

void DfsClientImpl::set_log_level(const char* level)
{
  //LOG(INFO, "set log level: %s", level);
  //LOGGER.setLogLevel(level);
}

void DfsClientImpl::set_log_file(const char* file)
{
  if (NULL == file)
  {
    //LOG(ERROR, "file is null");
  }
  else
  {
    //LOG(INFO, "set log file: %s", file);
  }
  //LOGGER.setFileName(file);
}

int32_t DfsClientImpl::get_block_cache_time() const
{
  int32_t ret = 0;
  if (NULL == default_dfs_session_)
  {
    //LOG(ERROR, "no default session");
  }
  else
  {
    ret = default_dfs_session_->get_cache_time();
  }
  return ret;
}

int32_t DfsClientImpl::get_block_cache_items() const
{
  int32_t ret = 0;
  if (NULL == default_dfs_session_)
  {
    //LOG(ERROR, "no default session");
  }
  else
  {
    ret = default_dfs_session_->get_cache_items();
  }
  return ret;
}

int32_t DfsClientImpl::get_cache_hit_ratio(CacheType cache_type) const
{
  return BgTask::get_cache_hit_ratio(cache_type);
}

uint64_t DfsClientImpl::get_server_id()
{
  uint64_t server_id = 0;
  if (default_dfs_session_ != NULL)
  {
    server_id = default_dfs_session_->get_ns_addr();
  }
  return server_id;
}

int32_t DfsClientImpl::get_cluster_id(const char* ns_addr)
{
  int32_t cluster_id = 0;
  if  (ns_addr != NULL)
  {
    DfsSession* dfs_session = NULL;
    if (!check_init())
    {
      //LOG(ERROR, "dfs client not init");
    }
    else if (NULL == (dfs_session = get_session(ns_addr)))
    {
      //LOG(ERROR, "can not get dfs session: %s.", NULL == ns_addr ? "default" : ns_addr);
    }
    else
    {
      cluster_id = dfs_session->get_cluster_id();
    }
  }
  else
  {
    if (default_dfs_session_ != NULL)
    {
      cluster_id = default_dfs_session_->get_cluster_id();
    }
  }
  return cluster_id;
}

int32_t DfsClientImpl::get_cluster_group_count(const char* ns_addr)
{
  int32_t cluster_group_count = -1;
  if  (ns_addr != NULL)
  {
    DfsSession* dfs_session = NULL;
    if (!check_init())
    {
      //LOG(ERROR, "dfs client not init");
    }
    else if (NULL == (dfs_session = get_session(ns_addr)))
    {
      //LOG(ERROR, "can not get dfs session: %s.", NULL == ns_addr ? "default" : ns_addr);
    }
    else
    {
      cluster_group_count = dfs_session->get_cluster_group_count_from_ns();
    }
  }
  else
  {
    if (default_dfs_session_ != NULL)
    {
      cluster_group_count = default_dfs_session_->get_cluster_group_count_from_ns();
    }
  }
  return cluster_group_count;
}

int32_t DfsClientImpl::get_cluster_group_seq(const char* ns_addr)
{
  int32_t cluster_group_seq = -1;
  if  (ns_addr != NULL)
  {
    DfsSession* dfs_session = NULL;
    if (!check_init())
    {
      //LOG(ERROR, "dfs client not init");
    }
    else if (NULL == (dfs_session = get_session(ns_addr)))
    {
      //LOG(ERROR, "can not get dfs session: %s.", NULL == ns_addr ? "default" : ns_addr);
    }
    else
    {
      cluster_group_seq = dfs_session->get_cluster_group_seq_from_ns();
    }
  }
  else
  {
    if (default_dfs_session_ != NULL)
    {
      cluster_group_seq = default_dfs_session_->get_cluster_group_seq_from_ns();
    }
  }
  return cluster_group_seq;
}

int64_t DfsClientImpl::save_buf(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                 const char* buf, const int64_t count,
                                 const int32_t flag, const char* suffix,
                                 const char* ns_addr, const char* key, const bool simple)
{
  int64_t ret = INVALID_FILE_SIZE;

  if (NULL == ret_dfs_name || ret_dfs_name_len < DFS_FILE_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else if (true == simple && ret_dfs_name_len < DFS_FILE_LEN + (int32_t)strlen(suffix))
  {
    //LOG(ERROR, "dfs name buffer space not enough.");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_buf_ex(ret_dfs_name, ret_dfs_name_len, buf, count, flag, NULL, suffix, ns_addr, key, simple);
  }

  return ret;
}

int64_t DfsClientImpl::save_file(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                 const char* local_file,
                                 const int32_t flag, const char* suffix,
                                 const char* ns_addr, const bool simple)
{
  int64_t ret = INVALID_FILE_SIZE;

  if (NULL == ret_dfs_name || ret_dfs_name_len < DFS_FILE_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else if (true == simple && ret_dfs_name_len < DFS_FILE_LEN + (int32_t)strlen(suffix))
  {
    //LOG(ERROR, "dfs name buffer space not enough.");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_file_ex(ret_dfs_name, ret_dfs_name_len, local_file, flag, NULL, suffix, ns_addr, simple);
 }
  return ret;
}

int64_t DfsClientImpl::save_file_update(const char* buf, const int64_t count, const int32_t flag,
                                        const char* file_name, const char* suffix,
                                        const char* ns_addr, const char* key)
{
  int64_t ret = INVALID_FILE_SIZE;
  if (NULL == file_name || static_cast<int32_t>(strlen(file_name)) < FILE_NAME_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_buf_ex(NULL, 0, buf, count, flag, file_name, suffix, ns_addr, key);
  }

  return ret;
}

int64_t DfsClientImpl::save_file_update(const char* local_file, const int32_t flag,
                                        const char* file_name, const char* suffix,
                                        const char* ns_addr)
{
  int64_t ret = INVALID_FILE_SIZE;
  if (NULL == file_name || static_cast<int32_t>(strlen(file_name)) < FILE_NAME_LEN)
  {
    //LOG(ERROR, "invalid parameter, must be return");
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    ret = save_file_ex(NULL, 0, local_file, flag, file_name, suffix, ns_addr);
  }

  return ret;
}

int64_t DfsClientImpl::save_file_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                    const char* local_file, const int32_t flag,
                                    const char* file_name, const char* suffix,
                                    const char* ns_addr, const bool simple)
{
  int ret = ERROR;
  int fd = -1;
  int64_t file_size = 0;

  if (NULL == local_file)
  {
    //LOG(ERROR, "local file is null");
  }
  else if ((flag & (~(T_DEFAULT|T_WRITE|T_NEWBLK|T_LARGE))) != 0)
  {
    //LOG(ERROR, "only T_DEFAULT or T_WRITE or T_NEWBLK or T_LARGE flag support to save");
  }
  else if ((fd = ::open(local_file, O_RDONLY)) < 0)
  {
    //LOG(ERROR, "open local file %s fail: %s", local_file, strerror(errno));
  }
  else
  {
    int dfs_fd = open(file_name, suffix, ns_addr, T_WRITE|flag, local_file);
    if (dfs_fd <= 0)
    {
      //LOG(ERROR, "open dfs file to write fail. dfsname: %s, suffix: %s, flag: %d, ret: %d",
      //          file_name, suffix, flag, dfs_fd);
    }
    else
    {
      int32_t io_size = MAX_READ_SIZE;
      if (flag & T_LARGE)
      {
        io_size = 4 * MAX_READ_SIZE;
      }

      char* buf = new char[io_size];
      int64_t read_len = 0, write_len = 0;

      while (true)
      {
        if ((read_len = ::read(fd, buf, io_size)) < 0)
        {
          if (EAGAIN == errno)
          {
            continue;
          }
          //LOG(ERROR, "read local file %s fail, ret: %"PRI64_PREFIX"d, error: %s", local_file, read_len, strerror(errno));
          break;
        }

        if (0 == read_len)
        {
          break;
        }

        if ((write_len = write(dfs_fd, buf, read_len)) != read_len)
        {
          //LOG(ERROR, "write to dfs fail, write len: %"PRI64_PREFIX"d, ret: %"PRI64_PREFIX"d",
          //          read_len, write_len);
          break;
        }

        file_size += read_len;

        if (read_len < MAX_READ_SIZE)
        {
          break;
        }
      }

      if ((ret = close(dfs_fd, ret_dfs_name, ret_dfs_name_len, simple)) != SUCCESS)
      {
        //LOG(ERROR, "close dfs file fail, ret: %d", ret);
      }

      if (SUCCESS == ret && true == simple)
      {
        // append suffix to returned name

        strncat(ret_dfs_name, suffix, strlen(suffix));
      }

      gDeleteA(buf);
    }

    ::close(fd);
  }

  return ret != SUCCESS ? INVALID_FILE_SIZE : file_size;
}

int64_t DfsClientImpl::save_buf_ex(char* ret_dfs_name, const int32_t ret_dfs_name_len,
                                    const char* buf, const int64_t count, const int32_t flag,
                                    const char* file_name, const char* suffix,
                                    const char* ns_addr,const char* key, const bool simple)
{
  int ret = ERROR;

  if (NULL == buf || count <= 0)
  {
    //LOG(ERROR, "invalid buffer and count. buffer: %p, count: %"PRI64_PREFIX"d", buf, count);
  }
  else if ((flag & (~(T_DEFAULT|T_WRITE|T_NEWBLK|T_LARGE))) != 0)
  {
    //LOG(ERROR, "only T_DEFAULT or T_WRITE or T_NEWBLK or T_LARGE flag support to save");
  }
  else
  {
    int dfs_fd = open(file_name, suffix, ns_addr, T_WRITE|flag, key);
    if (dfs_fd <= 0)
    {
      //LOG(ERROR, "open dfs file to write fail. dfsname: %s, suffix: %s, flag: %d, key: %s, ret: %d",
      //          file_name, suffix, flag, key, dfs_fd);
    }
    else
    {
      int64_t write_len = write(dfs_fd, buf, count);
      if (write_len != count)
      {
        //LOG(ERROR, "write to dfs fail, write len: %"PRI64_PREFIX"d, ret: %"PRI64_PREFIX"d",
        //          count, write_len);
      }

      // close anyway
      if ((ret = close(dfs_fd, ret_dfs_name, ret_dfs_name_len, simple)) != SUCCESS)
      {
        //LOG(ERROR, "close dfs file fail, ret: %d", ret);
      }

      if (SUCCESS == ret && true == simple)
      {
        // append suffix to returned name
        strncat(ret_dfs_name, suffix, strlen(suffix));
      }

    }
  }

  return ret != SUCCESS ? INVALID_FILE_SIZE : count;
}

int DfsClientImpl::fetch_file(const char* local_file, const char* file_name, const char* suffix, const char* ns_addr, const int flags)
{
  int ret = ERROR;
  int fd = -1;
  BaseFileType file_type = INVALID_DFS_FILE_TYPE;

  if (NULL == local_file)
  {
    //LOG(ERROR, "local file is null");
    ret = EXIT_PARAMETER_ERROR;
  }
  else if ((file_type = FSName::check_file_type(file_name)) == INVALID_DFS_FILE_TYPE)
  {
    //LOG(ERROR, "invalid dfs name: %s", file_name);
    ret = EXIT_PARAMETER_ERROR;
  }
  else if ((fd = ::open(local_file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0)
  {
    //LOG(ERROR, "open local file %s to write fail: %s", local_file, strerror(errno));
  }
  else
  {
    int32_t flag = T_DEFAULT | flags;
    int32_t io_size = MAX_READ_SIZE;
    if (file_type == LARGE_DFS_FILE_TYPE)
    {
      flag = T_LARGE;
      io_size = 4 * MAX_READ_SIZE;
    }

    int dfs_fd = this->open(file_name, suffix, ns_addr, T_READ|flag);
    if (dfs_fd <= 0)
    {
      //LOG(ERROR, "open dfs file to read fail. dfsname: %s, suffix: %s, ret: %d",
      //          file_name, suffix, dfs_fd);
    }
    else
    {
      char* buf = new char[io_size];
      int64_t read_len = 0, write_len = 0;

      while (1)
      {
        if ((read_len = this->read(dfs_fd, buf, io_size)) < 0)
        {
          //LOG(ERROR, "read dfs file fail. dfsname: %s, suffix: %s, ret: %"PRI64_PREFIX"d",
          //          file_name, suffix, read_len);
          break;
        }

        if (0 == read_len)
        {
          ret = SUCCESS;
          break;
        }

        if ((write_len = ::write(fd, buf, read_len)) != read_len)
        {
          //LOG(ERROR, "write local file %s fail, write len: %"PRI64_PREFIX"d, ret: %"PRI64_PREFIX"d, error: %s",
          //          local_file, read_len, write_len, strerror(errno));
          break;
        }

        if (read_len < io_size)
        {
          ret = SUCCESS;
          break;
        }
      }

      close(dfs_fd);
      gDeleteA(buf);
    }
    ::close(fd);
  }

  return ret;
}

int DfsClientImpl::fetch_file(int64_t& ret_count, char* buf, const int64_t count,
                              const char* file_name, const char* suffix,
                              const char* ns_addr, const int flags)
{
  int ret = ERROR;
  BaseFileType file_type = INVALID_DFS_FILE_TYPE;

  if (NULL == buf || count <= 0 || NULL == file_name)
  {
    //LOG(ERROR, "invalid parameter");
    ret = EXIT_PARAMETER_ERROR;
  }
  else if ((file_type = FSName::check_file_type(file_name)) == INVALID_DFS_FILE_TYPE)
  {
    //LOG(ERROR, "invalid dfs name: %s", file_name);
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    int32_t flag = T_DEFAULT | flags;
    int32_t io_size = MAX_READ_SIZE;
    if (file_type == LARGE_DFS_FILE_TYPE)
    {
      flag = T_LARGE;
      io_size = 4 * MAX_READ_SIZE;
    }

    int dfs_fd = this->open(file_name, suffix, ns_addr, T_READ|flag);
    if (dfs_fd <= 0)
    {
      //LOG(ERROR, "open dfs file to read fail. dfsname: %s, suffix: %s, ret: %d",
      //          file_name, suffix, dfs_fd);
    }
    else
    {
      int64_t read_len = 0, left_len = count, per_io_size = 0;
      while (left_len > 0)
      {
        per_io_size = io_size > left_len ? left_len : io_size;
        if ((read_len = this->read(dfs_fd, buf + (count - left_len), per_io_size)) < 0)
        {
          //LOG(ERROR, "read dfs file fail. dfsname: %s, suffix: %s, ret: %"PRI64_PREFIX"d",
          //          file_name, suffix, read_len);
          ret = read_len;
          break;
        }

        left_len -= read_len;

        // read over
        if (0 == left_len || read_len < per_io_size)
        {
          ret = SUCCESS;
          ret_count = count - left_len;
          break;
        }
      }

      close(dfs_fd);
    }
  }

  return ret;
}

int DfsClientImpl::fetch_file_ex(char*& buf, int64_t& count, const char* file_name, const char* suffix,
                                 const char* ns_addr)
{
  int ret = ERROR;
  BaseFileType file_type = INVALID_DFS_FILE_TYPE;

  if ((file_type = FSName::check_file_type(file_name)) == INVALID_DFS_FILE_TYPE)
  {
    //LOG(ERROR, "invalid dfs name: %s", file_name);
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    int32_t flag = T_DEFAULT;
    int32_t io_size = MAX_READ_SIZE;
    if (file_type == LARGE_DFS_FILE_TYPE)
    {
      flag = T_LARGE;
      io_size = 4 * MAX_READ_SIZE;
    }

    int dfs_fd = this->open(file_name, suffix, ns_addr, T_READ|flag);
    int64_t file_length = 0;

    if (dfs_fd <= 0)
    {
      //LOG(ERROR, "open dfs file to read fail. dfsname: %s, suffix: %s, ret: %d",
      //          file_name, suffix, dfs_fd);
    }
    else
    {
      if ((file_length = get_file_length(dfs_fd)) <= 0)
      {
        //LOG(ERROR, "get file length fail. ret: %"PRI64_PREFIX"d", file_length);
      }
      else if (file_length > DFS_MALLOC_MAX_SIZE) // cannot alloc buffer once
      {
        //LOG(ERROR, "file length larger than max malloc size. %"PRI64_PREFIX"d > %"PRI64_PREFIX"d",
        //          file_length, DFS_MALLOC_MAX_SIZE);
      }
      else
      {
        // user MUST free
        buf = new char[file_length];

        int64_t read_len = 0, left_len = file_length, per_io_size = 0;

        while (left_len > 0)
        {
          per_io_size = io_size > left_len ? left_len : io_size;
          if ((read_len = this->read(dfs_fd, buf + (file_length - left_len), per_io_size)) < 0)
          {
            //LOG(ERROR, "read dfs file fail. dfsname: %s, suffix: %s, ret: %"PRI64_PREFIX"d",
            //          file_name, suffix, read_len);
            ret = read_len;
            break;
          }

          if (left_len - read_len <= 0)
          {
            ret = SUCCESS;
            count = file_length;
            break;
          }

          left_len -= read_len;
        }

        if (SUCCESS != ret)
        {
          gDeleteA(buf);
        }
      }
      close(dfs_fd);
    }
  }

  return ret;
}

int DfsClientImpl::stat_file(DfsFileStat* file_stat, const char* file_name, const char* suffix,
                             const DfsStatType stat_type, const char* ns_addr)
{
  int ret = ERROR;
  BaseFileType file_type = INVALID_DFS_FILE_TYPE;

  if (NULL == file_stat)
  {
    //LOG(ERROR, "DfsFilestat is null");
    ret = EXIT_PARAMETER_ERROR;
  }
  else if ((file_type = FSName::check_file_type(file_name)) == INVALID_DFS_FILE_TYPE)
  {
    //LOG(ERROR, "invalid dfs name: %s", file_name);
    ret = EXIT_PARAMETER_ERROR;
  }
  else
  {
    int32_t flag = T_DEFAULT;
    if (LARGE_DFS_FILE_TYPE == file_type)
    {
      flag |= T_LARGE;
    }

    int dfs_fd = this->open(file_name, suffix, ns_addr, T_STAT|flag);
    if (dfs_fd < 0)
    {
      //LOG(ERROR, "open dfs file stat fail. dfsname: %s, suffix: %s", file_name, suffix);
    }
    else if ((ret = fstat(dfs_fd, file_stat, stat_type)) != SUCCESS)
    {
      //LOG(ERROR, "stat dfs file fail. dfsname: %s, suffix, %s, stattype: %d",
      //          file_name, suffix, stat_type);
    }
    this->close(dfs_fd);
  }

  return ret;
}

// check if dfsclient is already initialized.
// read and write and stuffs that need open first,
// need no init check cause open already does it,
// and they will check if file is open.
bool DfsClientImpl::check_init()
{
  if (!is_init_)
  {
    //LOG(ERROR, "dfsclient not initialized");
  }

  return is_init_;
}

DfsSession* DfsClientImpl::get_session(const char* ns_addr)
{
  return NULL == ns_addr ? default_dfs_session_ :
    SESSION_POOL.get(ns_addr, ClientConfig::cache_time_, ClientConfig::cache_items_);
}

DfsFile* DfsClientImpl::get_file(const int fd)
{
  Mutex::Lock lock(mutex_);
  FILE_MAP::iterator it = dfs_file_map_.find(fd);
  if (dfs_file_map_.end() == it)
  {
    //LOG(ERROR, "invaild fd, ret: %d", fd);
    return NULL;
  }
  return it->second;
}

int DfsClientImpl::get_fd()
{
  int ret_fd = EXIT_INVALIDFD_ERROR;

  Mutex::Lock lock(mutex_);
  if (static_cast<int32_t>(dfs_file_map_.size()) >= MAX_OPEN_FD_COUNT)
  {
    //LOG(ERROR, "too much open files");
  }
  else
  {
    if (MAX_FILE_FD == fd_)
    {
      fd_ = 0;
    }

    bool fd_confict = true;
    int retry = MAX_OPEN_FD_COUNT;

    while (retry-- > 0 &&
           (fd_confict = (dfs_file_map_.find(++fd_) != dfs_file_map_.end())))
    {
      if (MAX_FILE_FD == fd_)
      {
        fd_ = 0;
      }
    }

    if (fd_confict)
    {
      //LOG(ERROR, "too much open files");
    }
    else
    {
      ret_fd = fd_;
    }
  }

  return ret_fd;
}

int DfsClientImpl::insert_file(const int fd, DfsFile* dfs_file)
{
  int ret = ERROR;

  if (NULL != dfs_file)
  {
    Mutex::Lock lock(mutex_);
    ret = (dfs_file_map_.insert(std::map<int, DfsFile*>::value_type(fd, dfs_file))).second ?
      SUCCESS : ERROR;
  }

  return ret;
}

int DfsClientImpl::erase_file(const int fd)
{
  Mutex::Lock lock(mutex_);
  FILE_MAP::iterator it = dfs_file_map_.find(fd);
  if (dfs_file_map_.end() == it)
  {
    //LOG(ERROR, "invaild fd: %d", fd);
    return EXIT_INVALIDFD_ERROR;
  }
  gDelete(it->second);
  dfs_file_map_.erase(it);
  return SUCCESS;
}

bool DfsClientImpl::is_hit_local_cache(const char* ns_addr, const uint32_t block_id) const
{
  DfsSession *dfs_session = NULL;
  bool ret = false;

  if (ns_addr != NULL && strlen(ns_addr) > 0)
  {
    dfs_session = SESSION_POOL.get(ns_addr);
  }
  else
  {
    if (NULL != default_dfs_session_)
    {
      dfs_session = default_dfs_session_;
    }
  }

  if (NULL != dfs_session)
  {
    ret = dfs_session->is_hit_local_cache(block_id);
  }

  return ret;
}
