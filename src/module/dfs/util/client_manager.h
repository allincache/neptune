#ifndef N_DFS_UTIL_CLIENT_MANAGER_H_
#define N_DFS_UTIL_CLIENT_MANAGER_H_

#include <ext/hash_map>
#include "dfs/util/dfs.h"
#include "base/concurrent/Mutex.h"
#include "base_packet.h"
#include "base_packet_factory.h"
#include "base_packet_streamer.h"
#include "new_client.h"
#include "base/common/Internal.h"

namespace neptune {
namespace dfs {

class NewClientManager : public IPacketHandler
{
  friend int NewClient::post_request(const uint64_t server, Packet* packet, uint8_t& send_id);
  friend bool NewClient::async_wait();
  typedef __gnu_cxx::hash_map<uint32_t, NewClient*> NEWCLIENT_MAP;
  typedef NEWCLIENT_MAP::iterator NEWCLIENT_MAP_ITER;
  typedef int (*async_callback_func_entry)(NewClient* client, void* args);
 
 public:
  NewClientManager();
  virtual ~NewClientManager();
  static NewClientManager& get_instance()
  {
    static NewClientManager client_manager;
    return client_manager;
  }
  int initialize(BasePacketFactory* factory, BasePacketStreamer* streamer,
                Transport* transport = NULL, async_callback_func_entry entry = NULL,
                void* args = NULL);
  void destroy();
  bool is_init() const;
  IPacketHandler::HPRetCode handlePacket(Packet* packet, void* args);
  NewClient* create_client();
  bool destroy_client(NewClient* client);

  Packet* clone_packet(Packet* message, const int32_t version = DFS_PACKET_VERSION_V2, const bool deserialize = false);

  Packet* create_packet(const int32_t pcode);

  static NewClient* malloc_new_client_object(const uint32_t seq_id);
  static void free_new_client_object(NewClient* client);

 private:
  bool handlePacket(const WaitId& id, Packet* response);
  bool do_async_callback(NewClient* client);

 private:
  DISALLOW_COPY_AND_ASSIGN(NewClientManager);
  NEWCLIENT_MAP new_clients_;
  BasePacketFactory* factory_;
  BasePacketStreamer* streamer_;
  ConnectionManager* connmgr_;
  Transport* transport_;

  Mutex mutex_;
  async_callback_func_entry async_callback_entry_;
  void* args_;
  static const uint32_t MAX_SEQ_ID = 0xFFFFFF - 1;
  static const int32_t  DEFAULT_CLIENT_CONNTION_QUEUE_LIMIT = 256;
  uint32_t seq_id_;

  bool initialize_;
  bool own_transport_;
};

} //namespace dfs
} //namespace neptune

#endif
