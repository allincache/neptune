#include "dfs_rc_client_api_impl.h"
#include "dfs/util/func.h"
#include "dfs/util/session_util.h"
#include "dfs/util/client_manager.h"
#include "dfs_client_impl.h"
#include "dfs_rc_helper.h"
#include "base/fs/FsName.h"
#include "dfs_meta_client_api.h"

#define RC_CLIENT_VERSION "rc_1.0.0_c++"

namespace
{
  const int INIT_INVALID = 0;
  const int INIT_LOGINED = 1;
  const int CLUSTER_ACCESS_TYPE_READ_ONLY = 1;
  const int CLUSTER_ACCESS_TYPE_READ_WRITE = 2;
}

namespace neptune {
namespace dfs {

using namespace std;
using namespace neptune::dfs;

StatUpdateTask::StatUpdateTask(RcClientImpl& rc_client):rc_client_(rc_client)
{
}
void StatUpdateTask::runTimerTask()
{
uint64_t rc_ip = 0;
KeepAliveInfo ka_info;
{
  CThreadGuard mutex_guard(&rc_client_.mutex_);
  rc_client_.get_ka_info(ka_info);
  rc_ip = rc_client_.active_rc_ip_;
}
ka_info.s_stat_.cache_hit_ratio_ = DfsClientImpl::Instance()->get_cache_hit_ratio();
bool update_flag = false;
BaseInfo new_base_info;
int ret = RcHelper::keep_alive(rc_ip, ka_info, update_flag, new_base_info);
if (SUCCESS == ret)
{
  //LOG(DEBUG, "keep alive ok, update flag: %d", update_flag);
  {
    CThreadGuard mutex_guard(&rc_client_.mutex_);
    rc_client_.next_rc_index_ = 0;
  }
  int last_report_interval = 0;
  if (update_flag)
  {
    CThreadGuard mutex_guard(&rc_client_.mutex_);
    last_report_interval = rc_client_.base_info_.report_interval_;
    rc_client_.base_info_ = new_base_info;
    rc_client_.calculate_ns_info(new_base_info);
#ifdef WITH_TAIR_CACHE
    if (!rc_client_.ignore_rc_remote_cache_info_)
    {
      std::vector<std::string> ns_cache_info;
      Func::split_string(rc_client_.base_info_.ns_cache_info_.c_str(), ';', ns_cache_info);
      if (ns_cache_info.size() == 4)
      {
        DfsClientImpl::Instance()->set_remote_cache_info(ns_cache_info[0].c_str(),
            ns_cache_info[1].c_str(), ns_cache_info[2].c_str(),
            atoi(ns_cache_info[3].c_str()));
        DfsClientImpl::Instance()->set_use_remote_cache(rc_client_.base_info_.use_remote_cache_);
      }
      else
      {
        //LOG(WARN, "invalid ns_cache_info(size: %zd), remote cache will not initialize", ns_cache_info.size());
        DfsClientImpl::Instance()->set_use_remote_cache(false);
      }
    }
#endif
    rc_client_.session_base_info_.modify_time_ = rc_client_.base_info_.modify_time_;
  }
  if (update_flag && last_report_interval != new_base_info.report_interval_)
  {
    //LOG(DEBUG, "reschedule update stat task :old interval is %d new is %d",
    //    last_report_interval, new_base_info.report_interval_);

    rc_client_.keepalive_timer_->cancel(rc_client_.stat_update_task_);
    rc_client_.keepalive_timer_->scheduleRepeated(rc_client_.stat_update_task_,
        Time::seconds(new_base_info.report_interval_));
  }
}
else
{
  //LOG(DEBUG, "keep alive error will roll back");
  uint64_t next_rc_ip;
  CThreadGuard mutex_guard(&rc_client_.mutex_);
  next_rc_ip = rc_client_.get_active_rc_ip(rc_client_.next_rc_index_);
  if (0 == next_rc_ip)
  {
    rc_client_.next_rc_index_ = 0;
  }
  else
  {
    rc_client_.active_rc_ip_ = next_rc_ip;
  }
  // roll back stat info;
  rc_client_.stat_ += ka_info.s_stat_;
}
}

RcClientImpl::RcClientImpl()
:need_use_unique_(false), local_addr_(0),
init_stat_(INIT_INVALID), active_rc_ip_(0), next_rc_index_(0),
ignore_rc_remote_cache_info_(false), name_meta_client_(NULL), app_id_(0), my_fd_(1)
{
}

void RcClientImpl::destory()
{
if (0 != keepalive_timer_)
{
  keepalive_timer_->cancel(stat_update_task_);
  keepalive_timer_->destroy();
  stat_update_task_ = 0;
  keepalive_timer_ = 0;
}
if (NULL != name_meta_client_)
{
  delete name_meta_client_;
  name_meta_client_ = NULL;
}
std::map<int32_t, ClusterGroupInfo*>::iterator uit = update_ns_.begin();
for (; update_ns_.end() != uit; uit++)
{
  gDelete(uit->second);
}
update_ns_.clear();
}

RcClientImpl::~RcClientImpl()
{
destory();
DfsClientImpl::Instance()->destroy();
logout();
}
DfsRetType RcClientImpl::initialize(const char* str_rc_ip, const char* app_key, const char* str_app_ip,
  const int32_t cache_times, const int32_t cache_items, const char* dev_name, const char* rs_addr)
{
if (str_rc_ip == NULL || app_key == NULL)
{
  //LOG(WARN, "input parameter is invalid. rc_ip: %s, app_key: %s, app_ip: %s",
  //    str_rc_ip == NULL ? "null":str_rc_ip,
  //    app_key == NULL ? "null":app_key,
  //    str_app_ip == NULL ? "null":str_app_ip);
  return ERROR;
}
if (cache_times < 0 || cache_items < 0)
{
  //LOG(WARN, "invalid cache setting. cache_times: %d, cache_items: %d", cache_times, cache_items);
  return ERROR;
}
uint64_t rc_ip = Func::get_host_ip(str_rc_ip);
uint64_t app_ip = 0;
if (NULL != str_app_ip)
{
  app_ip = Func::str_to_addr(str_app_ip, 0);
}
return initialize(rc_ip, app_key, app_ip, cache_times, cache_items, dev_name, rs_addr);
}

DfsRetType RcClientImpl::initialize(const uint64_t rc_ip, const char* app_key, const uint64_t app_ip,
  const int32_t cache_times, const int32_t cache_items, const char* dev_name, const char* rs_addr)
{
int ret = SUCCESS;
CThreadGuard mutex_guard(&mutex_);
if (init_stat_ != INIT_LOGINED)
{
  destory();
  stat_update_task_ = new StatUpdateTask(*this);
  keepalive_timer_ = new Timer();
  name_meta_client_ = new NameMetaClient();
  if (SUCCESS == ret)
  {
    ret = DfsClientImpl::Instance()->initialize(NULL, cache_times, cache_items, true);
  }
  //LOG(DEBUG, "DfsClientImpl::Instance()->initialize ret %d", ret);
  if (SUCCESS == ret)
  {
    if (app_ip != 0)
    {
      local_addr_ = app_ip & 0xffffffff;
    }
    else
    {
      local_addr_ = Func::get_local_addr(dev_name);
    }
    //LOG(DEBUG, "local_addr_ = %s", CNetUtil::addrToString(local_addr_).c_str());
    ret = login(rc_ip, app_key, local_addr_);
  }
  //LOG(DEBUG, "login ret %d", ret);

  if (SUCCESS == ret)
  {
    session_base_info_.client_version_ = RC_CLIENT_VERSION;
    session_base_info_.cache_size_ = cache_items;
    session_base_info_.cache_time_ = cache_times;
    session_base_info_.modify_time_ = CTimeUtil::getTime();
    session_base_info_.is_logout_ = false;

    active_rc_ip_ = rc_ip;
    init_stat_ = INIT_LOGINED;
    if (NULL != rs_addr)
    {
      name_meta_client_->initialize(rs_addr);
    }
    else
    {
      name_meta_client_->initialize(base_info_.meta_root_server_);
    }
#ifdef WITH_TAIR_CACHE
    std::vector<std::string> ns_cache_info;
    Func::split_string(base_info_.ns_cache_info_.c_str(), ';', ns_cache_info);
    if (ns_cache_info.size() == 4)
    {
      DfsClientImpl::Instance()->set_remote_cache_info(ns_cache_info[0].c_str(),
          ns_cache_info[1].c_str(), ns_cache_info[2].c_str(),
          atoi(ns_cache_info[3].c_str()));
      DfsClientImpl::Instance()->set_use_remote_cache(base_info_.use_remote_cache_);
    }
    else
    {
      //LOG(WARN, "invalid ns_cache_info(size: %zd), remote cache will not initialize", ns_cache_info.size());
      DfsClientImpl::Instance()->set_use_remote_cache(false);
    }
#endif
    keepalive_timer_->scheduleRepeated(stat_update_task_,
        Time::seconds(base_info_.report_interval_));
  }
}
return ret;
}

DfsRetType RcClientImpl::logout()
{
int ret = ERROR;
size_t retry = 0;
uint64_t rc_ip = 0;
CThreadGuard mutex_guard(&mutex_);
ret = check_init_stat();
if (SUCCESS == ret)
{
  KeepAliveInfo ka_info;
  get_ka_info(ka_info);
  ka_info.s_base_info_.is_logout_ = true;
  while(0 != (rc_ip = get_active_rc_ip(retry)))
  {
    ret = RcHelper::logout(rc_ip, ka_info);
    if (SUCCESS == ret)
    {
      break;
    }
  }
}
if (SUCCESS == ret)
{
  init_stat_ = INIT_INVALID;
}
return ret;
}

#ifdef WITH_TAIR_CACHE
void RcClientImpl::set_remote_cache_info(const char * remote_cache_info)
{
std::vector<std::string> tair_addr;
Func::split_string(remote_cache_info, ';', tair_addr);
if (tair_addr.size() == 4)
{
  DfsClientImpl::Instance()->set_remote_cache_info(tair_addr[0].c_str(),
      tair_addr[1].c_str(), tair_addr[2].c_str(),
      atoi(tair_addr[3].c_str()));
  ignore_rc_remote_cache_info_ = true;
  DfsClientImpl::Instance()->set_use_remote_cache(true);
}
}
#endif

void RcClientImpl::set_client_retry_count(const int64_t count)
{
DfsClientImpl::Instance()->set_client_retry_count(count);
}

int64_t RcClientImpl::get_client_retry_count() const
{
return DfsClientImpl::Instance()->get_client_retry_count();
}

void RcClientImpl::set_client_retry_flag(bool retry_flag)
{
DfsClientImpl::Instance()->set_client_retry_flag(retry_flag);
}

void RcClientImpl::set_wait_timeout(const int64_t timeout_ms)
{
DfsClientImpl::Instance()->set_wait_timeout(timeout_ms);
}

void RcClientImpl::set_log_level(const char* level)
{
LOGGER.setLogLevel(level);
}

void RcClientImpl::set_log_file(const char* log_file)
{
LOGGER.setFileName(log_file);
}
int RcClientImpl::open(const char* file_name, const char* suffix, const RcClient::RC_MODE mode,
  const bool large, const char* local_key)
{
int fd = -1;
int ret = check_init_stat();
if (SUCCESS == ret)
{
  if ((RcClient::READ == mode) && NULL == file_name)
  {
    ret = ERROR;
  }
  else if (RcClient::READ == mode)
  {
    if ((*file_name == 'L' && false == large)
        || (*file_name == 'T' && true == large))
    {
      //LOG(WARN, "open a dfs file without right flag");
    }
  }
}

if (SUCCESS == ret)
{
  int flag = -1;
  ret = (RcClient::CREATE == mode && need_use_unique_) ? ERROR : SUCCESS;
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "should use save_file");
  }
  else//check mode
  {
    flag = -1;
    if (RcClient::CREATE == mode)
    {
      flag = T_WRITE;
    }
    else if (RcClient::READ == mode)
    {
      flag = T_READ | T_STAT;
    }
    else if(RcClient::READ_FORCE == mode)
    {
      flag = T_READ | T_STAT | T_FORCE;
    }

  }
  ret = flag != -1 ? SUCCESS : ERROR;
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "mode %d not support", mode);
  }
  else
  {
    if (have_permission(file_name, mode))
    {
      fdInfo fd_info(file_name, suffix, flag, large, local_key);
      fd = gen_fdinfo(fd_info);
    }
    else
    {
      //LOG(WARN, "no permission to do this");
    }
  }
}
return fd;
}

DfsRetType RcClientImpl::close(const int fd, char* dfs_name_buff, const int32_t buff_len)
{
int ret = check_init_stat();
if (SUCCESS == ret)
{
  fdInfo fd_info;
  remove_fdinfo(fd, fd_info);
  if (fd_info.raw_dfs_fd_ >= 0)
  {
    ret = DfsClientImpl::Instance()->close(fd_info.raw_dfs_fd_, dfs_name_buff, buff_len);
  }
}
return ret;
}

int64_t RcClientImpl::real_read(const int fd, const int raw_dfs_fd, void* buf, const int64_t count,
  fdInfo& fd_info, DfsFileStat* dfs_stat_buf)
{
int64_t read_count = -1;
if (raw_dfs_fd >= 0)
{
  int64_t start_time = CTimeUtil::getTime();
  DfsClientImpl::Instance()->lseek(raw_dfs_fd, fd_info.offset_, T_SEEK_SET);
  if (NULL != dfs_stat_buf)
  {
    read_count = DfsClientImpl::Instance()->readv2(raw_dfs_fd, buf, count, dfs_stat_buf);
  }
  else
  {
    ////LOG(DEBUG, "here offset is %d, raw_dfs_fd is %d", fd_info.offset_, raw_dfs_fd);
    read_count = DfsClientImpl::Instance()->read(raw_dfs_fd, buf, count);
  }
  if (read_count > 0)
  {
    fd_info.offset_ += read_count;
    // should use rc's fd, not raw_dfs_fd
    if (SUCCESS != update_fdinfo_offset(fd, fd_info.offset_))
    {
      //LOG(WARN, "update_fdinfo_offset error ");
    }
  }
  int64_t response_time = CTimeUtil::getTime() - start_time;
  add_stat_info(OPER_READ, read_count, response_time, read_count >= 0);
}
return read_count;
}
int64_t RcClientImpl::read_ex(const int fd, void* buf, const int64_t count, DfsFileStat* dfs_stat_buf )
{
int64_t read_count = -1;
int ret = check_init_stat();
if (SUCCESS == ret)
{
  fdInfo fd_info;
  ret = get_fdinfo(fd, fd_info);
  if (SUCCESS == ret)
  {
    if (fd_info.raw_dfs_fd_ >= 0)
    {
      read_count = real_read(fd, fd_info.raw_dfs_fd_, buf, count, fd_info, dfs_stat_buf);
    }
    else if (INVALID_RAW_DFS_FD == fd_info.raw_dfs_fd_)
    {
      //not open yet,
      int ns_get_index = 0;
      string ns_addr;
      int raw_dfs_fd = -1;
      RcClient::RC_MODE mode = RcClient::READ;
      const char* file_name = NULL;
      const char* suffix = NULL;
      const char* local_key = NULL;
      if (!fd_info.name_.empty())
      {
        file_name = fd_info.name_.c_str();
      }
      if (!fd_info.suffix_.empty())
      {
        suffix = fd_info.suffix_.c_str();
      }
      if (!fd_info.local_key_.empty())
      {
        local_key = fd_info.local_key_.c_str();
      }
      if (T_WRITE == fd_info.flag_)
      {
        mode = RcClient::CREATE;
      }
      do
      {
        ns_addr = get_ns_addr(file_name, mode, ns_get_index++);
        if (ns_addr.empty())
        {
          break;
        }
        raw_dfs_fd = open(ns_addr.c_str(), file_name, suffix,
            fd_info.flag_, fd_info.is_large_, local_key);
        read_count = real_read(fd, raw_dfs_fd, buf, count, fd_info, dfs_stat_buf);
        if (read_count < 0)
        {
          //LOG(WARN, "read file from ns %s error ret is %"PRI64_PREFIX"d",
          //    ns_addr.c_str(), read_count);
          if (raw_dfs_fd >= 0)
          {
            DfsClientImpl::Instance()->close(raw_dfs_fd);
          }
          raw_dfs_fd = -1;
        }
        else
        {
          break;
        }
      }while (raw_dfs_fd < 0);
      if (raw_dfs_fd >= 0)
      {
        if (SUCCESS != update_fdinfo_rawfd(fd, raw_dfs_fd))
        {
          DfsClientImpl::Instance()->close(raw_dfs_fd);
        }
      }
    }
    else
    {
      //LOG(ERROR, "name meta file not support read or readv2");
    }
  }
}
return read_count;
}
int64_t RcClientImpl::read(const int fd, void* buf, const int64_t count)
{
return read_ex(fd, buf, count, NULL);
}

int64_t RcClientImpl::readv2(const int fd, void* buf, const int64_t count, DfsFileStat* dfs_stat_buf)
{
return read_ex(fd, buf, count, dfs_stat_buf);
}

int64_t RcClientImpl::write(const int fd, const void* buf, const int64_t count)
{
int64_t write_count = -1;
int ret = check_init_stat();
if (SUCCESS == ret)
{
  fdInfo fd_info;
  ret = get_fdinfo(fd, fd_info);
  if (SUCCESS == ret)
  {
    int64_t start_time = CTimeUtil::getTime();
    if (fd_info.raw_dfs_fd_ >= 0)
    {
      write_count = DfsClientImpl::Instance()->write(fd_info.raw_dfs_fd_, buf, count);
    }
    else if (INVALID_RAW_DFS_FD == fd_info.raw_dfs_fd_)
    {
      int ns_get_index = 0;
      string ns_addr;
      int raw_dfs_fd = -1;
      RcClient::RC_MODE mode = RcClient::READ;
      const char* file_name = NULL;
      const char* suffix = NULL;
      const char* local_key = NULL;
      if (!fd_info.name_.empty())
      {
        file_name = fd_info.name_.c_str();
      }
      if (!fd_info.suffix_.empty())
      {
        suffix = fd_info.suffix_.c_str();
      }
      if (!fd_info.local_key_.empty())
      {
        local_key = fd_info.local_key_.c_str();
      }
      if (T_WRITE == fd_info.flag_)
      {
        mode = RcClient::CREATE;
      }
      do
      {
        ns_addr = get_ns_addr(file_name, mode, ns_get_index++);
        if (ns_addr.empty())
        {
          break;
        }
        raw_dfs_fd = open(ns_addr.c_str(), file_name, suffix,
            fd_info.flag_, fd_info.is_large_, local_key);
        write_count = DfsClientImpl::Instance()->write(raw_dfs_fd, buf, count);
        if (write_count < 0)
        {
          //LOG(WARN, "write file to ns %s error ret is %"PRI64_PREFIX"d",
          //    ns_addr.c_str(), write_count);
          if (raw_dfs_fd >= 0)
          {
            DfsClientImpl::Instance()->close(raw_dfs_fd);
          }
          raw_dfs_fd = -1;
        }
        else
        {
          break;
        }
      }while (raw_dfs_fd < 0);
      if (raw_dfs_fd >= 0)
      {
        if (SUCCESS != update_fdinfo_rawfd(fd, raw_dfs_fd))
        {
          DfsClientImpl::Instance()->close(raw_dfs_fd);
          write_count = -1;
        }
      }
    }
    else
    {
      //LOG(ERROR, "name meta file not support write");
    }
    int64_t response_time = CTimeUtil::getTime() - start_time;
    add_stat_info(OPER_WRITE, write_count, response_time, write_count >= 0);
  }
}
return write_count;
}

int64_t RcClientImpl::lseek(const int fd, const int64_t offset, const int whence)
{
int64_t ret_offset = -1;
fdInfo fd_info;
int ret = get_fdinfo(fd, fd_info);
if (SUCCESS == ret)
{
  if (fd_info.raw_dfs_fd_ >= 0 || INVALID_RAW_DFS_FD == fd_info.raw_dfs_fd_)
  {
    switch (whence)
    {
      case T_SEEK_SET:
        if (offset < 0)
        {
          //LOG(ERROR, "wrong offset seek_set, %"PRI64_PREFIX"d", offset);
          ret_offset = EXIT_PARAMETER_ERROR;
        }
        else
        {
          fd_info.offset_ = offset;
          ret_offset = fd_info.offset_;
        }
        break;
      case T_SEEK_CUR:
        if (fd_info.offset_ + offset < 0)
        {
          //LOG(ERROR, "wrong offset seek_cur, %"PRI64_PREFIX"d", offset);
          ret_offset = EXIT_PARAMETER_ERROR;
        }
        else
        {
          fd_info.offset_ += offset;
          ret_offset = fd_info.offset_;
        }
        break;
      default:
        //LOG(ERROR, "unknown seek flag: %d", whence);
        break;

    }
    if (ret_offset >= 0)
    {
      if (SUCCESS != update_fdinfo_offset(fd, ret_offset))
      {
        //LOG(WARN, "update_fdinfo_offset error ");
        ret_offset = -1;
      }
    }
  }
  else
  {
    //LOG(ERROR, "name meta file not support lseek");
  }
}
return ret_offset;
}

DfsRetType RcClientImpl::fstat(const int fd, DfsFileStat* buf, const DfsStatType fmode)
{
fdInfo fd_info;
int ret = get_fdinfo(fd, fd_info);
if (SUCCESS == ret)
{
  if (fd_info.raw_dfs_fd_ >= 0)
  {
    ret = DfsClientImpl::Instance()->fstat(fd_info.raw_dfs_fd_, buf, fmode);
  }
  else if (INVALID_RAW_DFS_FD == fd_info.raw_dfs_fd_)
  {
    int ns_get_index = 0;
    string ns_addr;
    int raw_dfs_fd = -1;
    RcClient::RC_MODE mode = RcClient::READ;
    const char* file_name = NULL;
    const char* suffix = NULL;
    const char* local_key = NULL;
    if (!fd_info.name_.empty())
    {
      file_name = fd_info.name_.c_str();
    }
    if (!fd_info.suffix_.empty())
    {
      suffix = fd_info.suffix_.c_str();
    }
    if (!fd_info.local_key_.empty())
    {
      local_key = fd_info.local_key_.c_str();
    }
    if (T_WRITE == fd_info.flag_)
    {
      mode = RcClient::CREATE;
    }
    do
    {
      ns_addr = get_ns_addr(file_name, mode, ns_get_index++);
      if (ns_addr.empty())
      {
        break;
      }
      raw_dfs_fd = open(ns_addr.c_str(), file_name, suffix,
          fd_info.flag_, fd_info.is_large_, local_key);
      ret = DfsClientImpl::Instance()->fstat(raw_dfs_fd, buf, fmode);
      if (SUCCESS != ret)
      {
        //LOG(WARN, "fstat file from ns %s error ret is %d",
        //    ns_addr.c_str(), ret);
        if (raw_dfs_fd >= 0)
        {
          DfsClientImpl::Instance()->close(raw_dfs_fd);
        }
        raw_dfs_fd = -1;
      }
      else
      {
        break;
      }
    }while (raw_dfs_fd < 0);
    if (raw_dfs_fd >= 0)
    {
      if (SUCCESS != update_fdinfo_rawfd(fd, raw_dfs_fd))
      {
        DfsClientImpl::Instance()->close(raw_dfs_fd);
      }
    }
  }
  else
  {
    //LOG(ERROR, "name meta file not support fstat");
  }
}
return ret;
}

DfsRetType RcClientImpl::unlink(const char* file_name, const char* suffix, const DfsUnlinkType action)
{
int ret = check_init_stat();
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(file_name, RcClient::CREATE, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    ret = unlink(ns_addr.c_str(), file_name, suffix, action);
  } while(SUCCESS != ret);
}
return ret;
}
DfsRetType RcClientImpl::unlink(const char* ns_addr, const char* file_name,
  const char* suffix, const DfsUnlinkType action)
{
int ret = SUCCESS;
if (need_use_unique_)
{
#ifdef WITH_UNIQUE_STORE
  ret = DfsClientImpl::Instance()->init_unique_store(duplicate_server_master_.c_str(),
      duplicate_server_slave_.c_str(),
      duplicate_server_group_.c_str(),
      duplicate_server_area_, ns_addr);
  if (SUCCESS == ret)
  {
    int64_t start_time = CTimeUtil::getTime();
    int64_t data_size = 0;
    int32_t ref_count = DfsClientImpl::Instance()->unlink_unique(data_size, file_name, suffix, ns_addr, 1);
    int64_t response_time = CTimeUtil::getTime() - start_time;
    add_stat_info(OPER_UNIQUE_UNLINK, data_size, response_time, ref_count >= 0);
    if (ref_count < 0)
    {
      ret = ERROR;
    }
  }
#else
  //LOG(ERROR, "you should compile client with --enable-uniquestore");
  ret = ERROR;
#endif
}
else
{
  int64_t start_time = CTimeUtil::getTime();
  int64_t data_size = 0;
  ret = DfsClientImpl::Instance()->unlink(data_size, file_name, suffix,
      ns_addr,action);
  int64_t response_time = CTimeUtil::getTime() - start_time;
  switch (action)
  {
    case DELETE:
      break;
    case UNDELETE:
      data_size = 0 - data_size;
      break;
    default:
      data_size = 0;
      break;
  }
  add_stat_info(OPER_UNLINK, data_size, response_time, ret == SUCCESS);
}
return ret;
}

int64_t RcClientImpl::save_file(const char* local_file, char* dfs_name_buff, const int32_t buff_len, const char* suffix, const bool is_large_file)
{
int ret = check_init_stat();
int64_t saved_size = -1;
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(dfs_name_buff, RcClient::CREATE, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    saved_size = save_file(ns_addr.c_str(), local_file, dfs_name_buff, buff_len, suffix, is_large_file);
  } while(saved_size < 0);
}
return saved_size;
}

int64_t RcClientImpl::save_buf(const char* source_data, const int32_t data_len,
  char* dfs_name_buff, const int32_t buff_len, const char* suffix)
{
int ret = check_init_stat();
int64_t saved_size = -1;
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(dfs_name_buff, RcClient::CREATE, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    saved_size = save_buf(ns_addr.c_str(), source_data, data_len,
        dfs_name_buff, buff_len, suffix);
  } while(saved_size < 0);
}
return saved_size;
}

int RcClientImpl::fetch_file(const char* local_file,
                  const char* file_name, const char* suffix)
{
int ret = check_init_stat();
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(file_name, RcClient::READ, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    ret = fetch_file(ns_addr.c_str(), local_file, file_name, suffix);
  } while(ret != SUCCESS);
}
return ret;
}

int RcClientImpl::fetch_buf(int64_t& ret_count, char* buf, const int64_t count,
                const char* file_name, const char* suffix)
{
int ret = check_init_stat();
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(file_name, RcClient::READ, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    ret = fetch_buf(ns_addr.c_str(), ret_count, buf, count, file_name, suffix);
  } while(ret != SUCCESS);
}
return ret;
}


bool RcClientImpl::is_hit_local_cache(const char* dfs_name)
{
int ret = check_init_stat();
bool bret = false;
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(dfs_name, RcClient::READ, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    bret = DfsClientImpl::Instance()->is_hit_local_cache(ns_addr.c_str(), dfs_name);
  } while(!bret);
}

return bret;
}

#ifdef WITH_TAIR_CACHE

bool RcClientImpl::is_hit_remote_cache(const char* dfs_name)
{
int ret = check_init_stat();
bool bret = false;
if (SUCCESS == ret)
{
  int ns_get_index = 0;
  string ns_addr;
  do
  {
    ns_addr = get_ns_addr(dfs_name, RcClient::READ, ns_get_index++);
    if (ns_addr.empty())
    {
      break;
    }
    bret = DfsClientImpl::Instance()->is_hit_remote_cache(ns_addr.c_str(), dfs_name);
  } while(!bret);
}

return bret;
}

#endif

int64_t RcClientImpl::save_file(const char* ns_addr, const char* local_file, char* dfs_name_buff,
  const int32_t buff_len, const char* suffix, const bool is_large_file)
{
int flag = T_DEFAULT;
if (is_large_file)
{
  flag = T_LARGE;
}
int64_t saved_size = -1;
if (need_use_unique_)
{
#ifdef WITH_UNIQUE_STORE
  int ret = DfsClientImpl::Instance()->init_unique_store(duplicate_server_master_.c_str(),
      duplicate_server_slave_.c_str(),
      duplicate_server_group_.c_str(),
      duplicate_server_area_, ns_addr);
  if (SUCCESS == ret)
  {
    int64_t start_time = CTimeUtil::getTime();
    saved_size = DfsClientImpl::Instance()->save_file_unique(dfs_name_buff, buff_len, local_file,
        suffix, ns_addr);
    int64_t response_time = CTimeUtil::getTime() - start_time;
    add_stat_info(OPER_UNIQUE_WRITE, saved_size, response_time, saved_size >= 0);
  }
#else
  //LOG(ERROR, "you should compile client with --enable-uniquestore");
  saved_size = -1;
#endif
}
else
{
  int64_t start_time = CTimeUtil::getTime();
  saved_size = DfsClientImpl::Instance()->save_file(dfs_name_buff, buff_len, local_file,
      flag, suffix, ns_addr);
  int64_t response_time = CTimeUtil::getTime() - start_time;
  add_stat_info(OPER_WRITE, saved_size, response_time, saved_size >= 0);
}
return saved_size;
}
int64_t RcClientImpl::save_buf(const char* ns_addr, const char* source_data, const int32_t data_len,
  char* dfs_name_buff, const int32_t buff_len, const char* suffix)
{
int64_t saved_size = -1;
if (need_use_unique_)
{
#ifdef WITH_UNIQUE_STORE
  int ret = DfsClientImpl::Instance()->init_unique_store(duplicate_server_master_.c_str(),
      duplicate_server_slave_.c_str(),
      duplicate_server_group_.c_str(),
      duplicate_server_area_, ns_addr);
  if (SUCCESS == ret)
  {
    int64_t start_time = CTimeUtil::getTime();
    saved_size = DfsClientImpl::Instance()->save_buf_unique(dfs_name_buff, buff_len, source_data, data_len,
        suffix, ns_addr);
    int64_t response_time = CTimeUtil::getTime() - start_time;
    add_stat_info(OPER_UNIQUE_WRITE, saved_size, response_time, saved_size >= 0);
  }
#else
  //LOG(ERROR, "you should compile client with --enable-uniquestore");
#endif
}
else
{
  int64_t start_time = CTimeUtil::getTime();
  saved_size = DfsClientImpl::Instance()->save_buf(dfs_name_buff, buff_len, source_data, data_len,
      T_DEFAULT, suffix, ns_addr);
  int64_t response_time = CTimeUtil::getTime() - start_time;
  add_stat_info(OPER_WRITE, saved_size, response_time, saved_size >= 0);
}
return saved_size;
}
int RcClientImpl::fetch_file(const char* ns_addr, const char* local_file,
  const char* file_name, const char* suffix)
{
int ret = SUCCESS;
int64_t start_time = CTimeUtil::getTime();
ret = DfsClientImpl::Instance()->fetch_file(local_file,
    file_name, suffix, ns_addr);
int64_t response_time = CTimeUtil::getTime() - start_time;
int64_t file_size = 0;
//TODO get file_size
add_stat_info(OPER_READ, file_size, response_time, SUCCESS == ret);
return ret;
}

int RcClientImpl::fetch_buf(const char* ns_addr, int64_t& ret_count, char* buf, const int64_t count,
  const char* file_name, const char* suffix)
{
int ret = SUCCESS;
int64_t start_time = CTimeUtil::getTime();
ret = DfsClientImpl::Instance()->fetch_file(ret_count, buf, count,
    file_name, suffix, ns_addr);
int64_t response_time = CTimeUtil::getTime() - start_time;
add_stat_info(OPER_READ, ret_count, response_time, SUCCESS == ret);
return ret;
}

DfsRetType RcClientImpl::login(const uint64_t rc_ip, const char* app_key, const uint64_t app_ip)
{
int ret = SUCCESS;
if (SUCCESS == (ret = RcHelper::login(rc_ip, app_key, app_ip,
        session_base_info_.session_id_, base_info_)))
{
  //LOG(DEBUG, "base_info_.ns_cache_info_ = %s", base_info_.ns_cache_info_.c_str());
  calculate_ns_info(base_info_);
  int32_t app_id = 0;
  int64_t session_ip = 0;
  SessionUtil::parse_session_id(session_base_info_.session_id_, app_id, session_ip);
  app_id_ = app_id;
}
return ret;
}

DfsRetType RcClientImpl::check_init_stat(const bool check_app_id) const
{
int ret = SUCCESS;
if (init_stat_ != INIT_LOGINED)
{
  //LOG(ERROR, "not inited");
  ret = ERROR;
}
if (check_app_id)
{
  if (app_id_ <= 0)
  {
    //LOG(ERROR, "app_id error");
    ret = ERROR;
  }
}
return ret;
}
uint64_t RcClientImpl::get_active_rc_ip(size_t& retry_index) const
{
uint64_t active_rc_ip = 0;
if (retry_index <= base_info_.rc_server_infos_.size())
{
  if (0 == retry_index)
  {
    active_rc_ip = active_rc_ip_;
  }
  else
  {
    active_rc_ip = base_info_.rc_server_infos_[retry_index - 1];
  }
  retry_index++;
}
return active_rc_ip;
}
void RcClientImpl::get_ka_info(KeepAliveInfo& kainfo)
{
kainfo.last_report_time_ = CTimeUtil::getTime();
kainfo.s_base_info_ = session_base_info_;
kainfo.s_stat_.app_oper_info_.swap(stat_.app_oper_info_);
}

void RcClientImpl::add_stat_info(const OperType& oper_type, const int64_t size,
  const int64_t response_time, const bool is_success)
{
AppOperInfo appinfo;
appinfo.oper_type_ = oper_type;
appinfo.oper_times_ = 1;
appinfo.oper_rt_ =  response_time;
if (is_success)
{
  appinfo.oper_size_ = size;
  appinfo.oper_succ_= 1;
}
CThreadGuard mutex_guard(&mutex_);
stat_.app_oper_info_[oper_type] += appinfo;
}

int RcClientImpl::open(const char* ns_addr, const char* file_name, const char* suffix,
  const int flag, const bool large, const char* local_key)
{
int ret = NULL == ns_addr ? -1 : 0;
if (0 == ret)
{
  int dfs_flag = large ? flag | T_LARGE : flag;
  ret = large ? DfsClientImpl::Instance()->open(file_name, suffix, ns_addr, dfs_flag, local_key)
              : DfsClientImpl::Instance()->open(file_name, suffix, ns_addr, dfs_flag);
}
return ret;
}

string RcClientImpl::get_ns_addr(const char* file_name, const RcClient::RC_MODE mode, const int index)
{
int32_t cluster_id = get_cluster_id(file_name);
uint32_t block_id = get_block_id(file_name);
return get_ns_addr_by_cluster_id(cluster_id, mode, index, block_id);

}
string RcClientImpl::get_ns_addr_by_cluster_id(int32_t cluster_id, const RcClient::RC_MODE mode, const int index, const uint32_t block_id)
{
string ns_addr;
if ((index >= CHOICE_CLUSTER_NS_TYPE_LENGTH)
    || (-1 == cluster_id && RcClient::CREATE != mode))
{
  //LOG(DEBUG, "wrong index or file not exist, index: %d, cluster_id: %d", index, cluster_id);
  //null ;
}
else
{
  CThreadGuard mutex_guard(&mutex_);
  if ((RcClient::READ == mode) && cluster_id == 0)
  {
    if (!choice[0].empty())
    {
      ns_addr = choice[0].begin()->second;
    }
  }
  else if (RcClient::CREATE == mode || RcClient::WRITE == mode)
  {
    if (0 == block_id)
    {
      ns_addr = write_ns_[index];
    }
    else // unlink
    {
      std::map<int32_t, ClusterGroupInfo*>::iterator it = update_ns_.find(cluster_id);
      if (it != update_ns_.end())
      {
        ClusterGroupInfo* cluster_group_info = it->second;
        bool bSuccess = cluster_group_info->get_ns_addr(block_id, ns_addr);
        if (!bSuccess)
        {
          bSuccess = update_cluster_group_info(cluster_group_info);
          if (bSuccess)
          {
            cluster_group_info->get_ns_addr(block_id, ns_addr);
          }
        }
      }
    }
  }
  else
  {
    ClusterNsType::const_iterator it = choice[index].find(cluster_id);
    if (it != choice[index].end())
    {
      ns_addr = it->second;
    }
  }
}
if (ns_addr.empty())
{
  ////LOG(INFO, "can not get ns_addr maybe you do not have access permition");
}
return ns_addr;
}

bool RcClientImpl::update_cluster_group_info(ClusterGroupInfo* cluster_group_info)
{
bool bRet = false;
std::vector<GroupInfo*> need_update_group_info_list;
cluster_group_info->get_need_update_group_info_list(need_update_group_info_list);
if (need_update_group_info_list.size() > 0)
{
  std::vector<GroupInfo*>::iterator iter = need_update_group_info_list.begin();
  for (; need_update_group_info_list.end() != iter; iter++)
  {
    int cluster_group_count = DfsClientImpl::Instance()->get_cluster_group_count((*iter)->ns_addr_.c_str());
    if (cluster_group_count > 0)
    {
      bRet = true;
      if (cluster_group_info->group_count_ > 0)
      {
        if (cluster_group_count != cluster_group_info->group_count_)
        {
          bRet = false;
          //LOG(ERROR, "cluster group count conflict. %d <==> %d", cluster_group_count, cluster_group_info->group_count_);
          break;
        }
      }
      else
      {
        cluster_group_info->group_count_ = cluster_group_count;
        //LOG(INFO, "set cluster group count %d for ns: %s", cluster_group_count, (*iter)->ns_addr_.c_str());
      }
      if (1 == cluster_group_count)
      {
        cluster_group_info->insert_group_info(0, (*iter)->ns_addr_, (*iter)->is_master_);
        //LOG(INFO, "set cluster group seq 0 for ns: %s, is_master: %d", (*iter)->ns_addr_.c_str(), (*iter)->is_master_);
      }
      else
      {
        int cluster_group_seq = DfsClientImpl::Instance()->get_cluster_group_seq((*iter)->ns_addr_.c_str());
        if (cluster_group_seq >= 0)
        {
          cluster_group_info->insert_group_info(cluster_group_seq, (*iter)->ns_addr_, (*iter)->is_master_);
          //LOG(INFO, "set cluster group seq %d for ns: %s, is_master: %d", cluster_group_seq, (*iter)->ns_addr_.c_str(), (*iter)->is_master_);
        }
        else
        {
          bRet = false;
          break;
        }
      }
    }
  }
}
return bRet;
}

int32_t RcClientImpl::get_cluster_id(const char* file_name)
{
FSName fs(file_name);
return fs.get_cluster_id();
}

uint32_t RcClientImpl::get_block_id(const char* file_name)
{
FSName fs(file_name);
return fs.get_block_id();
}

void RcClientImpl::calculate_ns_info(const BaseInfo& base_info)
{
for (int8_t i = 0; i < CHOICE_CLUSTER_NS_TYPE_LENGTH; ++i)
{
  write_ns_[i].clear();
  choice[i].clear();
}
std::map<int32_t, ClusterGroupInfo*>::iterator uit = update_ns_.begin();
for (; update_ns_.end() != uit; uit++)
{
  gDelete(uit->second);
}
update_ns_.clear();
need_use_unique_ = false;
std::vector<ClusterRackData>::const_iterator it = base_info.cluster_infos_.begin();
std::vector<ClusterData>::const_iterator cluster_data_it;
for (; it != base_info.cluster_infos_.end(); it++)
{
  //every cluster rack
  bool can_write = false;
  cluster_data_it = it->cluster_data_.begin();
  for (; cluster_data_it != it->cluster_data_.end(); cluster_data_it++)
  {
    //every cluster
    assert(0 != cluster_data_it->cluster_stat_);
    //rc server should not give the cluster which stat is 0
    assert(0 != cluster_data_it->access_type_);
    int32_t cluster_id = 0;
    bool is_master = false;
    parse_cluster_id(cluster_data_it->cluster_id_, cluster_id, is_master);

    if (CLUSTER_ACCESS_TYPE_READ_WRITE == cluster_data_it->access_type_)
    {
      can_write = true;
      add_ns_into_write_ns(cluster_data_it->ns_vip_);
    }
    add_ns_into_choice(cluster_data_it->ns_vip_, cluster_id);
  }
  if (can_write)
  {
    need_use_unique_ = it->need_duplicate_;
    if (need_use_unique_)
    {
      parse_duplicate_info(it->dupliate_server_addr_);
    }
  }
}
cluster_data_it = base_info_.cluster_infos_for_update_.begin();
// get updatable ns list
for (; cluster_data_it != base_info_.cluster_infos_for_update_.end(); cluster_data_it++)
{
  int32_t cluster_id = 0;
  bool is_master = false;
  parse_cluster_id(cluster_data_it->cluster_id_, cluster_id, is_master);

  add_ns_into_update_ns(cluster_data_it->ns_vip_, cluster_id, is_master);
}

//LOG(INFO, "need_use_unique_:%d", need_use_unique_);
for (int i = 0; i < CHOICE_CLUSTER_NS_TYPE_LENGTH; i++)
{
  //LOG(INFO, "%d write_ns %s", i, write_ns_[i].c_str());
  ClusterNsType::const_iterator it = choice[i].begin();
  for (; it != choice[i].end(); it++)
  {
    //LOG(INFO, "cluster_id :%d ns :%s", it->first, it->second.c_str());
  }
}

std::map<int32_t, ClusterGroupInfo*>::iterator git = update_ns_.begin();
//LOG(INFO, "update_ns: ");
for (; update_ns_.end() != git; git++)
{
  //LOG(INFO, "cluster_id: %d", git->first);
  ClusterGroupInfo* cluster_group_info = git->second;
  std::vector<GroupInfo*>::iterator iter = cluster_group_info->group_info_list_.begin();
  for (; cluster_group_info->group_info_list_.end() != iter; iter++)
  {
    //LOG(INFO, "group_seq: %d ns_addr :%s, is_master: %d", (*iter)->group_seq_, (*iter)->ns_addr_.c_str(), (*iter)->is_master_);
  }
}
return;
}

void RcClientImpl::parse_cluster_id(const std::string& cluster_id_str, int32_t& id, bool& is_master)
{
//cluster_id_str will be like 'T1M'  'T1B'
id = 0;
is_master = false;
if (cluster_id_str.length() < 3)
{
  //LOG(ERROR, "cluster_id_str error %s", cluster_id_str.c_str());
}
else
{
  id = cluster_id_str[1] - '0';
  is_master = (cluster_id_str[2] == 'M' || cluster_id_str[2] == 'm');
}
}
void RcClientImpl::parse_duplicate_info(const std::string& duplicate_info)
{
char tmp[512];
snprintf(tmp, 512, "%s", duplicate_info.c_str());
vector<char*> list;
CStringUtil::split(tmp, ";", list);
if (list.size() < 4)
{
  //LOG(ERROR, "parse_duplicate_info error: %s", duplicate_info.c_str());
}
else
{
  duplicate_server_master_ = list[0];
  duplicate_server_slave_ = list[1];
  duplicate_server_group_ = list[2];
  duplicate_server_area_ = atoi(list[3]);
}
//LOG(DEBUG, "master = %s slave = %s group= %s area = %d",
//    duplicate_server_master_.c_str(), duplicate_server_slave_.c_str(),
//    duplicate_server_group_.c_str(), duplicate_server_area_);
}

int RcClientImpl::add_ns_into_write_ns(const std::string& ip_str)
{
int32_t iret = !ip_str.empty() ? SUCCESS : ERROR;
if (SUCCESS == iret)
{
  int8_t index = 0;
  for (; index < CHOICE_CLUSTER_NS_TYPE_LENGTH; index++)
  {
    if (write_ns_[index].empty())
    {
      break;
    }
  }
  if (index < CHOICE_CLUSTER_NS_TYPE_LENGTH)
  {
    write_ns_[index] = ip_str;
  }
}
return iret;
}

int RcClientImpl::add_ns_into_choice(const std::string& ip_str, const int32_t cluster_id)
{
int32_t iret = !ip_str.empty() ? SUCCESS : ERROR;
if (SUCCESS == iret)
{
  int8_t index = 0;
  for (; index < CHOICE_CLUSTER_NS_TYPE_LENGTH; index++)
  {
    if (choice[index].empty())
    {
      break;
    }
    else
    {
      ClusterNsType::const_iterator it = choice[index].find(cluster_id);
      if (it == choice[index].end())
      {
        break;
      }
    }
  }

  if (index < CHOICE_CLUSTER_NS_TYPE_LENGTH)
  {
    choice[index][cluster_id] = ip_str;
  }
}
return iret;
}

int RcClientImpl::add_ns_into_update_ns(const std::string& ip_str, const int32_t cluster_id, bool is_master)
{
int32_t iret = !ip_str.empty() ? SUCCESS : ERROR;
if (SUCCESS == iret)
{
  std::map<int32_t, ClusterGroupInfo*>::iterator it = update_ns_.find(cluster_id);
  ClusterGroupInfo* cluster_group_info = NULL;
  if (update_ns_.end() == it)
  {
    cluster_group_info = new ClusterGroupInfo();
    update_ns_.insert(std::map<uint32_t, ClusterGroupInfo*>::value_type(cluster_id, cluster_group_info));
  }
  else
  {
    cluster_group_info = it->second;
  }
  cluster_group_info->insert_group_info(-1, ip_str, is_master);
  update_cluster_group_info(cluster_group_info);
}
return iret;
}

// for name meta
DfsRetType RcClientImpl::create_dir(const int64_t uid, const char* dir_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  ret = name_meta_client_->create_dir(app_id_, uid, dir_path);
}
return ret;
}

DfsRetType RcClientImpl::create_dir_with_parents(const int64_t uid, const char* dir_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  ret = name_meta_client_->create_dir_with_parents(app_id_, uid, dir_path);
}
return ret;
}

DfsRetType RcClientImpl::create_file(const int64_t uid, const char* file_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  ret = name_meta_client_->create_file(app_id_, uid, file_path);
}
return ret;
}

DfsRetType RcClientImpl::rm_dir(const int64_t uid, const char* dir_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  ret = name_meta_client_->rm_dir(app_id_, uid, dir_path);
}
return ret;
}
DfsRetType RcClientImpl::rm_file(const int64_t uid, const char* file_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  int cluster_id = name_meta_client_->get_cluster_id(app_id_, uid, file_path);
  if (-1 == cluster_id)
  {
    //LOG(DEBUG, "file not exsit, file_path: %s", file_path);
    ret = EXIT_TARGET_EXIST_ERROR;
  }
  else
  {
    // treat as write oper
    string ns_addr = get_ns_addr_by_cluster_id(cluster_id, RcClient::WRITE, 0);
    if (ns_addr.empty())
    {
      //LOG(ERROR, "can not do this operator in cluster %d", cluster_id);
    }
    else
    {
      ret = name_meta_client_->rm_file(ns_addr.c_str(), app_id_, uid, file_path);
    }
  }
}
return ret;
}

DfsRetType RcClientImpl::mv_dir(const int64_t uid, const char* src_dir_path,
  const char* dest_dir_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  ret = name_meta_client_->mv_dir(app_id_, uid, src_dir_path, dest_dir_path);
}
return ret;
}
DfsRetType RcClientImpl::mv_file(const int64_t uid, const char* src_file_path,
  const char* dest_file_path)
{
int ret = check_init_stat(true);
if (SUCCESS == ret)
{
  ret = name_meta_client_->mv_file(app_id_, uid, src_file_path, dest_file_path);
}
return ret;
}

bool RcClientImpl::have_permission(const char* file_name, const RcClient::RC_MODE mode)
{
return !get_ns_addr(file_name, mode, 0).empty(); //if we can not find a ns, we have no permission
}

bool RcClientImpl::is_raw_dfsname(const char* name)
{
  bool ret = false;
  if (NULL != name)
  {
    //TODO
    ret = (*name == 'T' || *name == 'L');
  }
  return ret;
}

DfsRetType RcClientImpl::ls_dir(const int64_t app_id, const int64_t uid, const char* dir_path,
    std::vector<FileMetaInfo>& v_file_meta_info)
{
  int ret = check_init_stat(true);
  if (SUCCESS == ret)
  {
    ret = name_meta_client_->ls_dir(app_id, uid, dir_path, v_file_meta_info);
  }
  return ret;
}

DfsRetType RcClientImpl::ls_file(const int64_t app_id, const int64_t uid,
    const char* file_path,
    FileMetaInfo& file_meta_info)
{
  int ret = check_init_stat(true);
  if (SUCCESS == ret)
  {
    ret = name_meta_client_->ls_file(app_id, uid, file_path, file_meta_info);
  }
  return ret;
}

bool RcClientImpl::is_dir_exist(const int64_t app_id, const int64_t uid, const char* dir_path)
{
  bool bRet = (SUCCESS == check_init_stat(true)) ? true : false;
  if (bRet)
  {
    bRet = name_meta_client_->is_dir_exist(app_id, uid, dir_path);
  }
  return bRet;
}

bool RcClientImpl::is_file_exist(const int64_t app_id, const int64_t uid, const char* file_path)
{
  bool bRet = (SUCCESS == check_init_stat(true)) ? true : false;
  if (bRet)
  {
    bRet = name_meta_client_->is_file_exist(app_id, uid, file_path);
  }
  return bRet;
}

int RcClientImpl::gen_fdinfo(const fdInfo& fdinfo)
{
  int gen_fd = -1;
  map<int, fdInfo>::iterator it;
  CThreadGuard mutex_guard(&fd_info_mutex_);
  if (static_cast<int32_t>(fd_infos_.size()) >= MAX_OPEN_FD_COUNT - 1)
  {
    //LOG(ERROR, "too much open files");
  }
  else
  {
    while(1)
    {
      if (MAX_FILE_FD == my_fd_)
      {
        my_fd_ = 1;
      }
      gen_fd = my_fd_++;
      it = fd_infos_.find(gen_fd);
      if (it == fd_infos_.end())
      {
        fd_infos_.insert(make_pair(gen_fd, fdinfo));
        break;
      }
    }
  }
  return gen_fd;
}

int RcClientImpl::open(const int64_t app_id, const int64_t uid, const char* name, const RcClient::RC_MODE mode)
{
  int dfs_ret = check_init_stat(true);
  int fd = -1;
  if (SUCCESS == dfs_ret)
  {
    if (RcClient::CREATE == mode && need_use_unique_)
    {
      //LOG(ERROR, "should use save_file");
      fd = -1;
    }
    else if ((RcClient::CREATE == mode || RcClient::WRITE == mode) && (app_id_ != app_id))
    {
      //LOG(ERROR, "can not write other app_id");
      fd = -1;
    }
    else
    {
      int32_t cluster_id = 0;
      cluster_id = name_meta_client_->get_cluster_id(app_id, uid, name);
      fdInfo fd_info(app_id, uid, cluster_id, mode, name);
      if (get_ns_addr_by_cluster_id(cluster_id, mode, 0).empty())
      {
        //LOG(ERROR, "can not do this operator in cluster %d", cluster_id);
      }
      else
      {
        fd = gen_fdinfo(fd_info);
        if (fd >= 0)
        {
          if (RcClient::CREATE == mode)
          {
            dfs_ret = name_meta_client_->create_file(app_id, uid, name);
            if (SUCCESS != dfs_ret)
            {
              remove_fdinfo(fd, fd_info);
              fd = -1;
            }
          }
        }
      }
    }
  }
  return fd;
}

int64_t RcClientImpl::pwrite(const int fd, const void* buf, const int64_t count, const int64_t offset)
{
  int64_t write_count = -1;
  int ret = check_init_stat();
  if (SUCCESS == ret)
  {
    fdInfo fd_info;
    ret = get_fdinfo(fd, fd_info);
    if (SUCCESS == ret)
    {
      int64_t start_time = CTimeUtil::getTime();
      if (NAME_DFS_FD == fd_info.raw_dfs_fd_)
      {
        int ns_get_index = 0;
        string ns_addr;
        RcClient::RC_MODE mode = (RcClient::RC_MODE)fd_info.flag_;
        ns_addr = get_ns_addr_by_cluster_id(fd_info.cluster_id_, mode, ns_get_index++);
        if (!ns_addr.empty())
        {
          write_count = name_meta_client_->write(ns_addr.c_str(), fd_info.app_id_, fd_info.uid_,
              fd_info.name_.c_str(), buf, offset, count);
        }
      }
      else
      {
        //LOG(WARN, "sorry, raw dfs client not support pwrite.");
        //write_count = DfsClientImpl::Instance()->pwrite(fd_info.raw_dfs_fd_, buf, count, offset);
      }
      int64_t response_time = CTimeUtil::getTime() - start_time;
      add_stat_info(OPER_WRITE, write_count, response_time, write_count >= 0);
    }
  }
  return write_count;
}

int64_t RcClientImpl::pread(const int fd, void* buf, const int64_t count, const int64_t offset)
{
  int ret = SUCCESS;
  int64_t read_count = -1;
  ret = check_init_stat();
  if (SUCCESS == ret)
  {
    fdInfo fd_info;
    ret = get_fdinfo(fd, fd_info);
    if (SUCCESS == ret)
    {
      int64_t start_time = CTimeUtil::getTime();
      if (NAME_DFS_FD == fd_info.raw_dfs_fd_)
      {
        //not open yet,
        int ns_get_index = 0;
        string ns_addr;
        RcClient::RC_MODE mode = (RcClient::RC_MODE)fd_info.flag_;
        do
        {
          ns_addr = get_ns_addr_by_cluster_id(fd_info.cluster_id_, mode, ns_get_index++);
          if (ns_addr.empty())
          {
            break;
          }
          read_count = name_meta_client_->read(ns_addr.c_str(), fd_info.app_id_, fd_info.uid_,
            fd_info.name_.c_str(), buf, offset, count);
          if (EXIT_TARGET_EXIST_ERROR == read_count ||
              EXIT_PARENT_EXIST_ERROR == read_count ||
              EXIT_INVALID_FILE_NAME == read_count )
          {
            //meta server error do not try other cluster
            break;
          }

          if (read_count < 0)
          {
            //LOG(WARN, "read file from ns %s error ret is %"PRI64_PREFIX"d",
            //    ns_addr.c_str(), read_count);
          }
        }while (read_count < 0);
      }
      else
      {
        //LOG(WARN, "sorry, raw dfs client not support pread.");
        //read_count = DfsClientImpl::Instance()->pread(fd_info.raw_dfs_fd_, buf, count, offset);
      }
      int64_t response_time = CTimeUtil::getTime() - start_time;
      add_stat_info(OPER_READ, read_count, response_time, read_count >= 0);
    }
  }
  return read_count;
}

int64_t RcClientImpl::save_file_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* local_file, const char* dfs_name)
{
  int64_t start_time = CTimeUtil::getTime();
  int64_t saved_size = name_meta_client_->save_file(ns_addr,
              app_id, uid, local_file, dfs_name);
  int64_t response_time = CTimeUtil::getTime() - start_time;
  add_stat_info(OPER_WRITE, saved_size, response_time, saved_size >= 0);
  return saved_size;
}

int64_t RcClientImpl::fetch_file_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* local_file, const char* dfs_name)
{
  int64_t start_time = CTimeUtil::getTime();
  int64_t fetched_size = name_meta_client_->fetch_file(ns_addr,
            app_id, uid, local_file, dfs_name);
    int64_t response_time = CTimeUtil::getTime() - start_time;
    add_stat_info(OPER_READ, fetched_size, response_time, fetched_size >= 0);
    return fetched_size;
}

int64_t RcClientImpl::save_buf_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
      const char* file_path, const char* buffer, const int64_t length)
{
  int64_t start_time = CTimeUtil::getTime();
  int64_t saved_size = name_meta_client_->write(ns_addr,
            app_id, uid, file_path, buffer, length);
  int64_t response_time = CTimeUtil::getTime() - start_time;
  add_stat_info(OPER_WRITE, saved_size, response_time, saved_size >= 0);
  return saved_size;
}

int64_t RcClientImpl::fetch_buf_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
      char* buffer, const int64_t offset, const int64_t length, const char* dfs_name)
{
  int64_t start_time = CTimeUtil::getTime();
  int64_t fetched_size = name_meta_client_->read(ns_addr,
            app_id, uid, dfs_name, buffer, offset, length);
  int64_t response_time = CTimeUtil::getTime() - start_time;
  add_stat_info(OPER_READ, fetched_size, response_time, fetched_size >= 0);
  return fetched_size;
}

int64_t RcClientImpl::save_file(const int64_t app_id, const int64_t uid,
    const char* local_file, const char* dfs_file_name)
{
  // dfs_file_name will be checked in sub interface
  int64_t saved_size = -1;
  int ret = check_init_stat();
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "RcClient not init");
  }
  else if (NULL == local_file || NULL == dfs_file_name || '/' != dfs_file_name[0])
  {
    ret = EXIT_INVALID_FILE_NAME;
    //LOG(ERROR, "invalid parameter");
  }
  else
  {
    // parse parent dir and create it
    char parent_dir[MAX_PATH_LENGTH];
    if (SUCCESS != (ret = Func::get_parent_dir(dfs_file_name, parent_dir, MAX_PATH_LENGTH)))
    {
      //LOG(ERROR, "get parent dir error: %s, ret: %d", dfs_file_name, ret);
    }
    else if (0 != strcmp("/", parent_dir) &&   // not root, and create dir fail
        SUCCESS != (ret = create_dir_with_parents(uid, parent_dir)) &&
        EXIT_TARGET_EXIST_ERROR != ret)
    {
      //LOG(ERROR, "create dir with parents error: %s, ret: %d", parent_dir, ret);
    }
    else
    {
      ret = SUCCESS;   // here ret may equal to EXIT_TARGET_EXIST_ERROR
      int ns_get_index = 0;
      string ns_addr;
      while (saved_size < 0)
      {
        ns_addr = get_ns_addr(NULL, RcClient::WRITE, ns_get_index++);
        if(ns_addr.empty())
        {
          break;
        }
        saved_size = save_file_ex(ns_addr.c_str(),
            app_id, uid, local_file, dfs_file_name);
      }
    }
  }
  return SUCCESS != ret? ret: saved_size;
}

int64_t RcClientImpl::save_buf(const int64_t app_id, const int64_t uid,
    const char* buf, const int64_t buf_len, const char* dfs_file_name)
{
  int64_t saved_size = -1;
  int ret = check_init_stat();
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "RcClient not init");
  }
  else if (NULL == dfs_file_name || '/' != dfs_file_name[0])
  {
    ret = EXIT_INVALID_FILE_NAME;
    //LOG(ERROR, "invalid parameter");
  }
  else
  {
    // parse parent dir and create it
    char parent_dir[MAX_PATH_LENGTH];
    if (SUCCESS != (ret = Func::get_parent_dir(dfs_file_name, parent_dir, MAX_PATH_LENGTH)))
    {
      //LOG(ERROR, "get parent dir error: %s, ret: %d", dfs_file_name, ret);
    }
    else if (0 != strcmp("/", parent_dir) &&   // not root, and create dir fail
        SUCCESS != (ret = create_dir_with_parents(uid, parent_dir)) &&
        EXIT_TARGET_EXIST_ERROR != ret)
    {
      //LOG(ERROR, "create dir with parents error: %s, ret: %d", parent_dir, ret);
    }
    else if (SUCCESS != (ret = create_file(uid, dfs_file_name)))
    {
      //LOG(ERROR, "create file error: %s, ret: %d", dfs_file_name, ret);
    }
    else
    {
      int ns_get_index = 0;
      string ns_addr;
      while (saved_size < 0)
      {
        ns_addr = get_ns_addr(NULL, RcClient::WRITE, ns_get_index++);
        if(ns_addr.empty())
        {
          break;
        }
        saved_size = save_buf_ex(ns_addr.c_str(),
                app_id, uid, dfs_file_name, buf, buf_len);
      }
    }
  }
  return SUCCESS != ret? ret: saved_size;
}

int64_t RcClientImpl::fetch_file(const int64_t app_id, const int64_t uid,
    const char* local_file, const char* dfs_file_name)
{
  int64_t fetched_size = -1;
  int ret = check_init_stat();
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "RcClient not init");
  }
  else if (NULL == local_file || NULL == dfs_file_name
      || '/' != dfs_file_name[0])
  {
    ret = EXIT_INVALID_FILE_NAME;
    //LOG(ERROR, "invalid parameter");
  }
  else
  {
    int ns_get_index = 0;
    string ns_addr;
    while (fetched_size < 0)
    {
      ns_addr = get_ns_addr(NULL, RcClient::READ, ns_get_index++);
      if(ns_addr.empty())
      {
        break;
      }
      fetched_size = fetch_file_ex(ns_addr.c_str(),
          app_id, uid, local_file, dfs_file_name);
    }
  }
  return SUCCESS != ret? ret: fetched_size;
}

int64_t RcClientImpl::fetch_buf(const int64_t app_id, const int64_t uid,
    char* buffer, const int64_t offset, const int64_t length, const char* dfs_file_name)
{
  int64_t fetched_size = -1;
  int ret = check_init_stat();
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "RcClient not init");
  }
  else if (NULL == dfs_file_name
      || '/' != dfs_file_name[0])
  {
    ret = EXIT_INVALID_FILE_NAME;
    //LOG(ERROR, "invalid parameter");
  }
  else if (NULL == buffer || length < 0 || offset < 0)
  {
    ret = EXIT_INVALID_ARGU;
    //LOG(ERROR, "invalid argument");
  }
  else
  {
    int ns_get_index = 0;
    string ns_addr;
    while (fetched_size < 0)
    {
      ns_addr = get_ns_addr(NULL, RcClient::READ, ns_get_index++);
      if(ns_addr.empty())
      {
        break;
      }
      fetched_size = fetch_buf_ex(ns_addr.c_str(),
          app_id, uid, buffer, offset, length, dfs_file_name);
    }
  }
  return SUCCESS != ret? ret: fetched_size;
}

DfsRetType RcClientImpl::remove_fdinfo(const int fd, fdInfo& fdinfo)
{
  DfsRetType ret = ERROR;
  map<int, fdInfo>::iterator it;
  CThreadGuard mutex_guard(&fd_info_mutex_);
  it = fd_infos_.find(fd);
  if (it != fd_infos_.end())
  {
    fdinfo = it->second;
    fd_infos_.erase(it);
    ret = SUCCESS;
  }
  return ret;
}
DfsRetType RcClientImpl::get_fdinfo(const int fd, fdInfo& fdinfo) const
{
  DfsRetType ret = ERROR;
  map<int, fdInfo>::const_iterator it;
  CThreadGuard mutex_guard(&fd_info_mutex_);
  it = fd_infos_.find(fd);
  if (it != fd_infos_.end())
  {
    fdinfo = it->second;
    ret = SUCCESS;
  }
  return ret;
}
DfsRetType RcClientImpl::update_fdinfo_offset(const int fd, const int64_t offset)
{
  DfsRetType ret = ERROR;
  map<int, fdInfo>::iterator it;
  CThreadGuard mutex_guard(&fd_info_mutex_);
  it = fd_infos_.find(fd);
  if (it != fd_infos_.end() && offset >= 0)
  {
    it->second.offset_ = offset;
    ret = SUCCESS;
  }
  return ret;
}
DfsRetType RcClientImpl::update_fdinfo_rawfd(const int fd, const int raw_fd)
{
  DfsRetType ret = ERROR;
  map<int, fdInfo>::iterator it;
  CThreadGuard mutex_guard(&fd_info_mutex_);
  it = fd_infos_.find(fd);
  if (it != fd_infos_.end() && INVALID_RAW_DFS_FD == it->second.raw_dfs_fd_)
  {
    it->second.raw_dfs_fd_= raw_fd;
    ret = SUCCESS;
  }
  return ret;
}

}
}
