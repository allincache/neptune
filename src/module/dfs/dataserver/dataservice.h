#ifndef N_DFS_DS_DATASERVICE_H_
#define N_DFS_DS_DATASERVICE_H_

#include <string>
#include "base/time/Timer.h"
#include "base/concurrent/Mutex.h"
#include "base/common/Internal.h"
#include "dfs/util/base_packet.h"
#include "dfs/util/base_service.h"
#include "dfs/util/statistics.h"
#include "dfs/util/status_message.h"
#include "dfs/message/message_factory.h"
#include "replicate_block.h"
#include "compact_block.h"
#include "check_block.h"
#include "sync_base.h"
#include "visit_stat.h"
#include "cpu_metrics.h"
#include "data_management.h"
#include "requester.h"
#include "block_checker.h"
#include "gc.h"


namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::dfs;
using namespace neptune::dfs;

#define WRITE_STAT_LOGGER write_stat_log_
#define WRITE_STAT_PRINT(level, ...) WRITE_STAT_LOGGER.logMessage(//LOG_LEVEL(level), __VA_ARGS__)
#define WRITE_STAT_LOG(level, ...) (//LOG_LEVEL_##level>WRITE_STAT_LOGGER._level) ? (void)0 : WRITE_STAT_PRINT(level, __VA_ARGS__)

#define READ_STAT_LOGGER read_stat_log_
#define READ_STAT_PRINT(level, ...) READ_STAT_LOGGER.logMessage(//LOG_LEVEL(level), __VA_ARGS__)
#define READ_STAT_LOG(level, ...) (//LOG_LEVEL_##level>READ_STAT_LOGGER._level) ? (void)0 : READ_STAT_PRINT(level, __VA_ARGS__)
    
class DataService: public BaseService
{

  friend int SyncBase::run_sync_mirror();

  public:
    DataService();

    virtual ~DataService();

    /** application parse args*/
    virtual int parse_common_line_args(int argc, char* argv[], std::string& errmsg);

    /** get listen port*/
    virtual int get_listen_port() const ;

    /** initialize application data*/
    virtual int initialize(int argc, char* argv[]);

    /** destroy application data*/
    virtual int destroy_service();

    /** create the packet streamer, this is used to create packet according to packet code */
    virtual IPacketStreamer* create_packet_streamer()
    {
      return new BasePacketStreamer();
    }

    /** destroy the packet streamer*/
    virtual void destroy_packet_streamer(IPacketStreamer* streamer)
    {
      gDelete(streamer);
    }

    /** create the packet streamer, this is used to create packet*/
    virtual BasePacketFactory* create_packet_factory()
    {
      return new MessageFactory();
    }

    /** destroy packet factory*/
    virtual void destroy_packet_factory(BasePacketFactory* factory)
    {
      gDelete(factory);
    }

    /** handle single packet */
    virtual IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet);

    /** handle packet*/
    virtual bool handlePacketQueue(Packet *packet, void *args);

    bool check_response(NewClient* client);
    int callback(NewClient* client);

    int post_message_to_server(BasePacket* message, const VUINT64& ds_list);

    int stop_heart();

    std::string get_real_work_dir();

  protected:
    virtual const char* get_log_file_path();
    virtual const char* get_pid_file_path();

  private:
    int run_heart(const int32_t who);
    int run_check();

    int create_file_number(CreateFilenameMessage* message);
    int write_data(WriteDataMessage* message);
    int close_write_file(CloseFileMessage* message);

    int write_raw_data(WriteRawDataMessage* message);
    int batch_write_info(WriteInfoBatchMessage* message);

    int read_data(ReadDataMessage* message);
    int read_data_extra(ReadDataMessageV2* message, int32_t version);
    int read_raw_data(ReadRawDataMessage* message);
    int read_file_info(FileInfoMessage* message);

    int rename_file(RenameFileMessage* message);
    int unlink_file(UnlinkFileMessage* message);

    //NS <-> DS
    int new_block(NewBlockMessage* message);
    int remove_block(RemoveBlockMessage* message);

    int replicate_block_cmd(ReplicateBlockMessage* message);
    int compact_block_cmd(CompactBlockMessage* message);
    int crc_error_cmd(CrcErrorMessage* message);

    //get single blockinfo
    int get_block_info(GetBlockInfoMessage* message);

    int query_bit_map(ListBitMapMessage* message);
    //get blockinfos on this server
    int list_blocks(ListBlockMessage* message);
    int reset_block_version(ResetBlockVersionMessage* message);

    int get_server_status(GetServerStatusMessage* message);
    int get_ping_status(StatusMessage* message);
    int client_command(ClientCmdMessage* message);

    int reload_config(ReloadConfigMessage* message);
    int send_blocks_to_ns(int8_t& heart_interval, const int32_t who, const int64_t timeout);
    int send_blocks_to_ns(BasePacket* packet);

    int get_dataserver_information(BasePacket* packet);

    // check modified blocks
    int check_blocks(BasePacket* packet);

  private:
    bool access_deny(BasePacket* message);
    void do_stat(const uint64_t peer_id,
        const int32_t visit_file_size, const int32_t real_len, const int32_t offset, const int32_t mode);
    int set_ns_ip();
    void try_add_repair_task(const uint32_t block_id, const int ret);
    int init_log_file(CLogger& logger, const std::string& log_file);
    int init_sync_mirror();

  private:
  class HeartBeatThreadHelper: public Thread
  {
    public:
      HeartBeatThreadHelper(DataService& service, const int32_t who):
          service_(service),
          who_(who)
      {
        start();
      }
      virtual ~HeartBeatThreadHelper(){}
      void run();
    private:
      DISALLOW_COPY_AND_ASSIGN(HeartBeatThreadHelper);
      DataService& service_;
      int32_t who_;
  };
  typedef Handle<HeartBeatThreadHelper> HeartBeatThreadHelperPtr;

  class DoCheckThreadHelper: public Thread
  {
    public:
      explicit DoCheckThreadHelper(DataService& service):
          service_(service)
      {
        start();
      }
      virtual ~DoCheckThreadHelper(){}
      void run();
    private:
      DISALLOW_COPY_AND_ASSIGN(DoCheckThreadHelper);
      DataService& service_;
  };
  typedef Handle<DoCheckThreadHelper> DoCheckThreadHelperPtr;

  class ReplicateBlockThreadHelper: public Thread
  {
    public:
      explicit ReplicateBlockThreadHelper(DataService& service):
          service_(service)
      {
        start();
      }
      virtual ~ReplicateBlockThreadHelper(){}
      void run();
    private:
      DISALLOW_COPY_AND_ASSIGN(ReplicateBlockThreadHelper);
      DataService& service_;
  };
  typedef Handle<ReplicateBlockThreadHelper> ReplicateBlockThreadHelperPtr;

  class CompactBlockThreadHelper: public Thread
  {
    public:
      explicit CompactBlockThreadHelper(DataService& service):
          service_(service)
      {
        start();
      }
      virtual ~CompactBlockThreadHelper(){}
      void run();
    private:
      DISALLOW_COPY_AND_ASSIGN(CompactBlockThreadHelper);
      DataService& service_;
  };
  typedef Handle<CompactBlockThreadHelper> CompactBlockThreadHelperPtr;

  private:
    DISALLOW_COPY_AND_ASSIGN(DataService);

    DataServerStatInfo data_server_info_; //dataserver info
    std::string server_index_;
    DataManagement data_management_;
    Requester ds_requester_;
    BlockChecker block_checker_;

    int32_t server_local_port_;
    bool need_send_blockinfo_[2];
    bool set_flag_[2];
    uint64_t hb_ip_port_[2];
    uint64_t ns_ip_port_; //metaserver ip port;

    ReplicateBlock* repl_block_; //replicate
    CompactBlock* compact_block_; //compact
    CheckBlock* check_block_;  // check
#if defined(NEP_GTEST)
  public:
#else
#endif
    std::vector<SyncBase*> sync_mirror_;
    int32_t sync_mirror_status_;

    Mutex stop_mutex_;
    Mutex client_mutex_;
    Mutex compact_mutext_;
    Mutex count_mutex_;
    Mutex read_stat_mutex_;
    Mutex sync_mirror_mutex_;

    AccessControl acl_;
    AccessStat acs_;
    VisitStat visit_stat_;
    CpuMetrics cpu_metrics_;
    int32_t max_cpu_usage_;

    //write and read log
    CLogger write_stat_log_;
    CLogger read_stat_log_;
    std::vector<std::pair<uint32_t, uint64_t> > read_stat_buffer_;
    static const unsigned READ_STAT_LOG_BUFFER_LEN = 100;

    //global stat
    TimerPtr timer_;
    StatManager<std::string, std::string, StatEntry > stat_mgr_;
    std::string dfs_ds_stat_;

    HeartBeatThreadHelperPtr heartbeat_thread_[2];
    DoCheckThreadHelperPtr   do_check_thread_;
    ReplicateBlockThreadHelperPtr* replicate_block_threads_;
    CompactBlockThreadHelperPtr  compact_block_thread_;

    std::string read_stat_log_file_;
    std::string write_stat_log_file_;
};

}
}
}

#endif //N_DFS_DS_DATASERVICE_H_
