#ifndef N_BASE_NET_UDPCOMPONENT_H_
#define N_BASE_NET_UDPCOMPONENT_H_

namespace neptune {
namespace base {

class UDPComponent : public IOComponent {

 public:
  UDPComponent(Transport *owner, Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter);
  ~UDPComponent();

  bool init(bool isServer = false);
  void close();
  bool handleWriteEvent();
  bool handleReadEvent();

 private:
  __gnu_cxx::hash_map<int, UDPConnection*> _connections;  // UDP���Ӽ���
  IPacketStreamer *_streamer;                             // streamer
  IServerAdapter *_serverAdapter;                         // serveradapter
};

}//namespace base
}//namespace neptune

#endif
