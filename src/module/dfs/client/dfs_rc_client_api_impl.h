#ifndef N_DFS_CLIENT_RC_CLIENTAPI_IMPL_H_
#define N_DFS_CLIENT_RC_CLIENTAPI_IMPL_H_

#include <stdio.h>
#include "dfs/util/dfs.h"
#include "base/time/Timer.h"
#include "dfs/util/rc_define.h"
#include "dfs_rc_client_api.h"

namespace neptune {
  
class FileMetaInf;
  
namespace dfs {

class RcClientImpl;
class NameMetaClient;
class StatUpdateTask : public TimerTask
{
  public:
    explicit StatUpdateTask(RcClientImpl& rc_client);
    virtual void runTimerTask();
  private:
    RcClientImpl& rc_client_;
};
typedef Handle<StatUpdateTask> StatUpdateTaskPtr;
class RcClientImpl
{
  friend class StatUpdateTask;
  public:
    RcClientImpl();
    ~RcClientImpl();

    DfsRetType initialize(const char* str_rc_ip, const char* app_key, const char* str_app_ip,
        const int32_t cache_times = DEFAULT_BLOCK_CACHE_TIME,
        const int32_t cache_items = DEFAULT_BLOCK_CACHE_ITEMS,
        const char* dev_name = NULL,
        const char* rs_addr = NULL);
    //return value :SUCCESS/ERROR;
    DfsRetType initialize(const uint64_t rc_ip, const char* app_key, const uint64_t app_ip,
        const int32_t cache_times = DEFAULT_BLOCK_CACHE_TIME,
        const int32_t cache_items = DEFAULT_BLOCK_CACHE_ITEMS,
        const char* dev_name = NULL,
        const char* rs_addr = NULL);

    int64_t get_app_id() const { return app_id_;}
#ifdef WITH_TAIR_CACHE
    void set_remote_cache_info(const char * remote_cache_info);
#endif
    void set_client_retry_count(const int64_t count);
    int64_t get_client_retry_count() const;
    void set_client_retry_flag(bool retry_flag);

    void set_wait_timeout(const int64_t timeout_ms);
    void set_log_level(const char* level);
    void set_log_file(const char* log_file);

    DfsRetType logout();

    // for raw dfs
    int open(const char* file_name, const char* suffix, RcClient::RC_MODE mode,
        const bool large = false, const char* local_key = NULL);
    DfsRetType close(const int fd, char* dfs_name_buff = NULL, const int32_t buff_len = 0);

    int64_t read(const int fd, void* buf, const int64_t count);
    int64_t readv2(const int fd, void* buf, const int64_t count, DfsFileStat* dfs_stat_buf);

    int64_t write(const int fd, const void* buf, const int64_t count);

    int64_t lseek(const int fd, const int64_t offset, const int whence);
    DfsRetType fstat(const int fd, DfsFileStat* buf, const DfsStatType fmode = NORMAL_STAT);

    DfsRetType unlink(const char* file_name, const char* suffix = NULL,
        const DfsUnlinkType action = DELETE);

    int64_t save_file(const char* local_file, char* dfs_name_buff, const int32_t buff_len,
        const char* suffix = NULL, const bool is_large_file = false);
    int64_t save_buf(const char* source_data, const int32_t data_len,
        char* dfs_name_buff, const int32_t buff_len, const char* suffix = NULL);

    int fetch_file(const char* local_file,
                    const char* file_name, const char* suffix = NULL);
    int fetch_buf(int64_t& ret_count, char* buf, const int64_t count,
                  const char* file_name, const char* suffix = NULL);

    bool is_hit_local_cache(const char* dfs_name);

#ifdef WITH_TAIR_CACHE
    bool is_hit_remote_cache(const char* dfs_name);
#endif

    // for name meta
    DfsRetType create_dir(const int64_t uid, const char* dir_path);
    DfsRetType create_dir_with_parents(const int64_t uid, const char* dir_path);
    DfsRetType create_file(const int64_t uid, const char* file_path);

    DfsRetType rm_dir(const int64_t uid, const char* dir_path);
    DfsRetType rm_file(const int64_t uid, const char* file_path);

    DfsRetType mv_dir(const int64_t uid, const char* src_dir_path, const char* dest_dir_path);
    DfsRetType mv_file(const int64_t uid, const char* src_file_path, const char* dest_file_path);

    DfsRetType ls_dir(const int64_t app_id, const int64_t uid, const char* dir_path,
        std::vector<FileMetaInfo>& v_file_meta_info);
    DfsRetType ls_file(const int64_t app_id, const int64_t uid,
        const char* file_path,
        FileMetaInfo& file_meta_info);

    bool is_dir_exist(const int64_t app_id, const int64_t uid, const char* dir_path);
    bool is_file_exist(const int64_t app_id, const int64_t uid, const char* file_path);

    int open(const int64_t app_id, const int64_t uid, const char* name, const RcClient::RC_MODE mode);
    int64_t pread(const int fd, void* buf, const int64_t count, const int64_t offset);
    int64_t pwrite(const int fd, const void* buf, const int64_t count, const int64_t offset);
    //use the same close func as raw dfs

    int64_t save_file(const int64_t app_id, const int64_t uid,
        const char* local_file, const char* dfs_file_name);

    int64_t save_buf(const int64_t app_id, const int64_t uid,
      const char* buf, const int64_t buf_len, const char* dfs_file_name);

    int64_t fetch_file(const int64_t app_id, const int64_t uid,
        const char* local_file, const char* dfs_file_name);

    int64_t fetch_buf(const int64_t app_id, const int64_t uid,
      char* buffer, const int64_t offset, const int64_t length, const char* dfs_file_name);

  private:
    DISALLOW_COPY_AND_ASSIGN(RcClientImpl);

  private:
    DfsRetType login(const uint64_t rc_ip, const char* app_key, const uint64_t app_ip);

    DfsRetType check_init_stat(const bool check_app_id = false) const;

    void destory();


    uint64_t get_active_rc_ip(size_t& retry_index) const;
    void get_ka_info(KeepAliveInfo& kainfo);

    void add_stat_info(const OperType& oper_type, const int64_t size,
        const int64_t response_time, const bool is_success);

    int open(const char* ns_addr, const char* file_name, const char* suffix,
        const int flag, const bool large, const char* local_key);

    DfsRetType unlink(const char* ns_addr, const char* file_name,
        const char* suffix, const DfsUnlinkType action);

    int64_t save_file(const char* ns_addr, const char* local_file, char* dfs_name_buff,
        const int32_t buff_len, const char* suffix = NULL, const bool is_large_file = false);

    int64_t save_buf(const char* ns_addr, const char* source_data, const int32_t data_len,
        char* dfs_name_buff, const int32_t buff_len, const char* suffix = NULL);

    int fetch_file(const char* ns_addr, const char* local_file,
                    const char* file_name, const char* suffix);

    int fetch_buf(const char* ns_addr, int64_t& ret_count, char* buf, const int64_t count,
                  const char* file_name, const char* suffix);

    std::string get_ns_addr(const char* file_name, const RcClient::RC_MODE mode, const int index);
    std::string get_ns_addr_by_cluster_id(int32_t cluster_id, const RcClient::RC_MODE mode, const int index, const uint32_t block_id = 0);

    static int32_t get_cluster_id(const char* file_name);
    static uint32_t get_block_id(const char* file_name);
    static void parse_cluster_id(const std::string& cluster_id_str, int32_t& id, bool& is_master);

    void calculate_ns_info(const BaseInfo& base_info);

    void parse_duplicate_info(const std::string& duplicate_info);

    int64_t save_file_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
        const char* local_file, const char* dfs_name);

    int64_t fetch_file_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
        const char* local_file, const char* dfs_name);

    int64_t save_buf_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
        const char* file_path, const char* buffer, const int64_t length);

    int64_t fetch_buf_ex(const char* ns_addr, const int64_t app_id, const int64_t uid,
        char* buffer, const int64_t offset, const int64_t length, const char* dfs_name);

  public:

    int add_ns_into_write_ns(const std::string& ip_str);

    int add_ns_into_choice(const std::string& ip_str, const int32_t cluster_id);

    int add_ns_into_update_ns(const std::string& ip_str, const int32_t cluster_id, bool is_master);

  private:
    static const int8_t CHOICE_CLUSTER_NS_TYPE_LENGTH = 3;
    typedef std::map<int32_t, std::string> ClusterNsType; //<cluster_id, ns>
    ClusterNsType choice[CHOICE_CLUSTER_NS_TYPE_LENGTH];
    std::string write_ns_[CHOICE_CLUSTER_NS_TYPE_LENGTH];
    struct GroupInfo
    {
      GroupInfo():group_seq_(-1), is_master_(false)
      {
      }
      GroupInfo(const int group_seq, const std::string& ns_addr, const bool is_master):
        group_seq_(group_seq), ns_addr_(ns_addr), is_master_(is_master)
      {
      }
      int group_seq_;
      std::string ns_addr_;
      bool is_master_;
    };
    struct ClusterGroupInfo
    {
      ClusterGroupInfo():group_count_(-1)
      {
      }
      ~ClusterGroupInfo()
      {
        std::vector<GroupInfo*>::iterator iter = group_info_list_.begin();
        for (; group_info_list_.end() != iter; iter++)
        {
          delete (*iter);
          //gDelete(*iter);
        }
      }
      void insert_group_info(const int group_seq, const std::string& ns_addr, const bool is_master)
      {
        std::vector<GroupInfo*>::iterator iter = group_info_list_.begin();
        for (; group_info_list_.end() != iter; iter++)
        {
          // exist
          if (!(*iter)->ns_addr_.compare(ns_addr))
          {
            (*iter)->group_seq_ = group_seq;
            break;
          }
        }
        // new insert
        if (group_info_list_.end() == iter)
        {
          GroupInfo* group_info = new GroupInfo(group_seq, ns_addr, is_master);
          if (is_master)
          {
            group_info_list_.insert(group_info_list_.begin(), group_info);
          }
          else
          {
            group_info_list_.push_back(group_info);
          }
        }
      }
      void get_need_update_group_info_list(std::vector<GroupInfo*>& need_group_info_list)
      {
        std::vector<GroupInfo*>::iterator iter = group_info_list_.begin();
        for (; group_info_list_.end() != iter; iter++)
        {
          if (-1 == (*iter)->group_seq_)
          {
            need_group_info_list.push_back(*iter);
          }
        }
      }
      bool get_ns_addr(const uint32_t block_id, std::string& ns_addr)
      {
        bool bRet = false;
        if (group_count_ > 0)
        {
          int group_seq = block_id % group_count_;
          std::vector<GroupInfo*>::iterator iter = group_info_list_.begin();
          for (; group_info_list_.end() != iter; iter++)
          {
            if (group_seq == (*iter)->group_seq_)
            {
              ns_addr = (*iter)->ns_addr_;
              bRet = true;
              break;
            }
          }
        }
        return bRet;
      }
      int group_count_;
      std::vector<GroupInfo*> group_info_list_;
    };
    std::map<int32_t, ClusterGroupInfo*> update_ns_; //<cluster_id, cluster_group_info>
    std::string duplicate_server_master_;
    std::string duplicate_server_slave_;
    std::string duplicate_server_group_;
    int32_t duplicate_server_area_;
    bool need_use_unique_;
    uint32_t local_addr_;

  public:
    bool update_cluster_group_info(ClusterGroupInfo* cluster_group_info);

  private:
    int32_t init_stat_;
    uint64_t active_rc_ip_;
    size_t next_rc_index_;
    bool ignore_rc_remote_cache_info_;

    BaseInfo base_info_;
    SessionBaseInfo session_base_info_;
    SessionStat stat_;

    TimerPtr keepalive_timer_;
    StatUpdateTaskPtr stat_update_task_;

  private:
    mutable CThreadMutex mutex_;

  private:
    enum {
      INVALID_RAW_DFS_FD = -1,
      NAME_DFS_FD = -2,
    };
    struct fdInfo
    {
      fdInfo()
        :raw_dfs_fd_(INVALID_RAW_DFS_FD), flag_(0),
        app_id_(0), uid_(0), offset_(0), is_large_(false), cluster_id_(0)
      {
      }
      fdInfo(const char* file_name, const char* suffix, const int flag,
          const bool large, const char* local_key)
        :raw_dfs_fd_(INVALID_RAW_DFS_FD), flag_(flag), app_id_(0), uid_(0),
        offset_(0), is_large_(large), cluster_id_(0)
      {
        if (NULL != file_name)
        {
          name_ = file_name;
        }
        if (NULL != suffix)
        {
          suffix_ = suffix;
        }
        if (NULL != local_key)
        {
          local_key_ = local_key;
        }
      }
      fdInfo(const int64_t app_id, const int64_t uid, const int32_t cluster_id, const int flag,
          const char* name = NULL)
        :raw_dfs_fd_(NAME_DFS_FD), flag_(flag), app_id_(app_id), uid_(uid), offset_(0), is_large_(false),cluster_id_(cluster_id)
      {
        if (NULL != name)
        {
          name_ = name;
        }
      }
      int raw_dfs_fd_;
      int flag_;
      int64_t app_id_;
      int64_t uid_;
      int64_t offset_;
      bool is_large_;
      int cluster_id_;
      std::string local_key_;
      std::string name_;
      std::string suffix_;
      std::string ns_addr_;
    };
    std::map<int, fdInfo> fd_infos_;
    mutable CThreadMutex fd_info_mutex_;
    NameMetaClient* name_meta_client_;
    int64_t app_id_;
    int my_fd_;
  private:
    bool have_permission(const char* file_name, const RcClient::RC_MODE mode);
    static bool is_raw_dfsname(const char* name);
    int gen_fdinfo(const fdInfo& fdinfo);
    DfsRetType remove_fdinfo(const int fd, fdInfo& fdinfo);
    DfsRetType get_fdinfo(const int fd, fdInfo& fdinfo) const;
    DfsRetType update_fdinfo_offset(const int fd, const int64_t offset);
    DfsRetType update_fdinfo_rawfd(const int fd, const int raw_fd);
    int64_t real_read(const int fd, const int raw_dfs_fd, void* buf, const int64_t count,
        fdInfo& fd_info, DfsFileStat* dfs_stat_buf);
    int64_t read_ex(const int fd, void* buf, const int64_t count, DfsFileStat* dfs_stat_buf);

};


}
}

#endif
