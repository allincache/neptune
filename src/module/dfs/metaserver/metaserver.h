#ifndef N_DFS_MS_NAMESERVER_H_
#define N_DFS_MS_NAMESERVER_H_

#include "base/common/Internal.h"
#include "dfs/util/base_service.h"
#include "ns_define.h"
#include "layout_manager.h"
#include "heart_manager.h"

namespace neptune {
namespace dfs {
namespace metaserver {

    using namespace neptune::dfs;
    using namespace neptune::dfs;

    class MetaServer;
    /*class OwnerCheckTimerTask: public TimerTask
    {
    public:
      explicit OwnerCheckTimerTask(MetaServer& manager);
      virtual ~OwnerCheckTimerTask();
      virtual void runTimerTask();
    private:
      DISALLOW_COPY_AND_ASSIGN( OwnerCheckTimerTask);
      MetaServer& manager_;
      const int64_t MAX_LOOP_TIME;
      int64_t max_owner_check_time_;
      int64_t owner_check_time_;
      int32_t main_task_queue_size_;
    };
    typedef Handle<OwnerCheckTimerTask> OwnerCheckTimerTaskPtr;*/

    class MetaServer: public BaseService
    {
    public:
      MetaServer();
      virtual ~MetaServer();
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

      int callback(NewClient* client);

      LayoutManager& get_layout_manager() { return layout_manager_;}
      HeartManagement& get_heart_management() { return heart_manager_;}

   private:
      DISALLOW_COPY_AND_ASSIGN(MetaServer);
      LayoutManager layout_manager_;
      MetaServerHeartManager master_slave_heart_manager_;
      HeartManagement heart_manager_;
    protected:
      /** get log file path*/
      virtual const char* get_log_file_path() { return NULL;}

      /** get pid file path */
      virtual const char* get_pid_file_path() { return NULL;}

    private:
      int open(BasePacket* msg);
      int close(BasePacket* msg);
      int batch_open(BasePacket* msg);
      int update_block_info(BasePacket* msg);
      int show_server_information(BasePacket* msg);
      //int owner_check(BasePacket* msg);
      int ping(BasePacket* msg);
      int dump_plan(BasePacket* msg);
      int client_control_cmd(BasePacket* msg);
      int do_master_msg_helper(BasePacket* packet);
      int do_slave_msg_helper(BasePacket* packet);

      int initialize_ns_global_info();
    };

}
}
}

#endif
