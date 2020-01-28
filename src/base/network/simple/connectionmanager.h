#ifndef N_BASE_NET_CONNECTION_MANAGER_H_
#define N_BASE_NET_CONNECTION_MANAGER_H_

#include <ext/hash_map>
//using namespace __gnu_cxx;

namespace neptune {
namespace base {

typedef __gnu_cxx::hash_map<uint64_t, Connection*, __gnu_cxx::hash<int> > NEP_NET_CONN_MAP;

class ConnectionManager {
 public:
  ConnectionManager(Transport *transport, IPacketStreamer *streamer, IPacketHandler *packetHandler);

  ~ConnectionManager();

  Connection *connect(uint64_t serverId, IPacketHandler *packetHandler, int queueLimit, int queueTimeout);
  
  void disconnect(uint64_t serverId);

  void setDefaultQueueLimit(uint64_t serverId, int queueLimit);

  void setDefaultQueueTimeout(uint64_t serverId, int queueTimeout);

  void setDefaultPacketHandler(uint64_t serverId, IPacketHandler *packetHandler);

  bool sendPacket(uint64_t serverId, Packet *packet, IPacketHandler *packetHandler = NULL, void *args = NULL, bool noblocking = true);

  void cleanup();

  Connection *getConnection(uint64_t serverId);

  static bool isAlive(uint64_t serverId);

 private:
  Transport *_transport;
  IPacketStreamer *_streamer;
  IPacketHandler *_packetHandler;
  int _queueLimit;
  int _queueTimeout;
  int _status;

  NEP_NET_CONN_MAP _connectMap;
  CThreadMutex _mutex;
};

} //namespace base
} //namespace neptune

#endif

