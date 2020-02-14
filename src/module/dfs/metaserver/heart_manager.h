#ifndef N_DFS_MS_HEART_MANAGEMENT_
#define N_DFS_MS_HEART_MANAGEMENT_

#include "base/time/Timer.h"
#include "dfs/message/message_factory.h"

namespace neptune {
namespace dfs {
namespace metaserver {

class MetaServer;
class HeartManagement
{
 public:
  explicit HeartManagement(MetaServer& manager);
  virtual ~HeartManagement();
  int initialize(const int32_t keepalive_thread_count, const int32_t report_block_thread_count);
  void wait_for_shut_down();
  void destroy();
  int push(BasePacket* msg);

 private:
  class KeepAliveIPacketQueueHeaderHelper : public IPacketQueueHandler
  {
   public:
    explicit KeepAliveIPacketQueueHeaderHelper(HeartManagement& manager): manager_(manager){};
    virtual ~KeepAliveIPacketQueueHeaderHelper() {}
    virtual bool handlePacketQueue(Packet* packet, void *args);
   
   private:
    DISALLOW_COPY_AND_ASSIGN(KeepAliveIPacketQueueHeaderHelper);
    HeartManagement& manager_;
  };

  class ReportBlockIPacketQueueHeaderHelper: public IPacketQueueHandler
  {
   public:
    explicit ReportBlockIPacketQueueHeaderHelper(HeartManagement& manager): manager_(manager){};
    virtual ~ReportBlockIPacketQueueHeaderHelper(){}
    virtual bool handlePacketQueue(Packet* packet, void *args);
  
   private:
    DISALLOW_COPY_AND_ASSIGN(ReportBlockIPacketQueueHeaderHelper);
    HeartManagement& manager_;
  };

 private:
  DISALLOW_COPY_AND_ASSIGN(HeartManagement);
  int keepalive(Packet* packet);
  int report_block(Packet* packet);
  MetaServer& manager_;
  PacketQueueThread keepalive_threads_;
  PacketQueueThread report_block_threads_;
  KeepAliveIPacketQueueHeaderHelper keepalive_queue_header_;
  ReportBlockIPacketQueueHeaderHelper report_block_queue_header_;
};

class MetaServerHeartManager: public IPacketQueueHandler
{
  friend class MetaServer;
 
 public:
  explicit MetaServerHeartManager(LayoutManager& manager);
  virtual ~MetaServerHeartManager();
  int initialize();
  int wait_for_shut_down();
  int destroy();
  int push(BasePacket* message, const int32_t max_queue_size = 0, const bool block = false);
  virtual bool handlePacketQueue(Packet *packet, void *args);
 
 private:
  class CheckThreadHelper : public Thread
  {
    public:
      explicit CheckThreadHelper(MetaServerHeartManager& manager): manager_(manager) { start();}
      virtual ~CheckThreadHelper() {}
      void run();
    private:
      MetaServerHeartManager& manager_;
      DISALLOW_COPY_AND_ASSIGN(CheckThreadHelper);
  };
  typedef Handle<CheckThreadHelper> CheckThreadHelperPtr;

 private:
  int keepalive_(BasePacket* message);
  int keepalive_(int32_t& sleep_time, NsKeepAliveType& type, NsRuntimeGlobalInformation& ngi);
  void check_();
  bool check_vip_(const NsRuntimeGlobalInformation& ngi) const;
  int ns_role_establish_(NsRuntimeGlobalInformation& ngi, const time_t now);
  int establish_peer_role_(NsRuntimeGlobalInformation& ngi);
  int ns_check_lease_expired_(NsRuntimeGlobalInformation& ngi, const time_t now);

  void switch_role_master_to_slave_(NsRuntimeGlobalInformation& ngi, const time_t now);
  void switch_role_salve_to_master_(NsRuntimeGlobalInformation& ngi, const time_t now);

  int keepalive_in_heartbeat_(BasePacket* message);
 
 private:
  LayoutManager& manager_;
  CheckThreadHelperPtr check_thread_;
  PacketQueueThread work_thread_;
};

}
}
}

#endif

