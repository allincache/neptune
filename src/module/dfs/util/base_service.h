#ifndef N_DFS_UTIL_BASE_SERVICE_H_
#define N_DFS_UTIL_BASE_SERVICE_H_

#include "dfs/util/dfs.h"
#include "base/time/Timer.h"
#include "dfs/util/Service.h"
#include "base_main.h"
#include "base_packet.h"
#include "base_packet_factory.h"
#include "base_packet_streamer.h"

namespace neptune {
namespace dfs {

class NewClient;
class BaseService :  public BaseMain,
                      public IServerAdapter,
                      public IPacketQueueHandler 
{
 public:
  BaseService();
  virtual ~BaseService();

  /** golbal aysnc callback function*/
  static int golbal_async_callback_func(NewClient* client, void* args);

  inline BasePacketFactory* get_packet_factory() { return packet_factory_;}

  /** get the packete streamer */
  inline IPacketStreamer* get_packet_streamer() { return  streamer_;}

  /** get transport*/
  Transport* get_transport() const { return transport_;}

  /** stop this service*/
  bool destroy();

  /** get timer*/
  inline TimerPtr& get_timer() { return timer_;}

  /** handle single packet */
  virtual IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet);

  /** handle packet*/
  virtual bool handlePacketQueue(Packet *packet, void *args);

  /** get listen port*/
  virtual int get_listen_port() const { return get_port();}

  /** initialize application data*/
  virtual int initialize(int, char* []) { return SUCCESS;}

  /** destroy application data*/
  virtual int destroy_service() {return SUCCESS;}

  /** create the packet streamer, this is used to create packet according to packet code */
  virtual IPacketStreamer* create_packet_streamer() = 0;

  /** destroy the packet streamer*/
  virtual void destroy_packet_streamer(IPacketStreamer* streamer) = 0;

  /** create the packet factory, this is used to create packet*/
  virtual BasePacketFactory* create_packet_factory() = 0;

  /** destroy packet factory*/
  virtual void destroy_packet_factory(BasePacketFactory* factory) = 0;

  /** async callback function*/
  virtual int async_callback(NewClient* client, void* args);

  /** push workitem to workers*/
  bool push(BasePacket* packet, bool block = false);

  /** get listen port*/
  int32_t get_port() const;

  /** get network device*/
  const char* const get_dev() const;

  /** get main work thread count*/
  int32_t get_work_thread_count() const;

  /** get work queue size */
  int32_t get_work_queue_size() const;

  /** get ip addr*/
  const char* get_ip_addr() const;

private:
  /** application main entry*/
  virtual int run(int argc , char*argv[]);

  /** initialize tbnet*/
  int initialize_network(const char* app_name);

 private:
  BasePacketFactory* packet_factory_;
  BasePacketStreamer* streamer_;
  TimerPtr timer_;

 protected:
  Transport* transport_;
  PacketQueueThread main_workers_;
  int32_t work_queue_size_;
};

} //namespace dfs
} //namespace neptune

#endif
