#ifndef N_BASE_NET_TCPCOMPONENT_H_
#define N_BASE_NET_TCPCOMPONENT_H_

namespace neptune {
namespace base {

class TCPComponent : public IOComponent {
 public:
  TCPComponent(Transport *owner, Socket *socket,
                IPacketStreamer *streamer, IServerAdapter *serverAdapter);

  ~TCPComponent();

  bool init(bool isServer = false);

  void close();

  bool handleWriteEvent();

  bool handleReadEvent();

  TCPConnection *getConnection() {
      return _connection;
  }

  void checkTimeout(int64_t now);

  bool socketConnect();

 private:
  TCPConnection *_connection;
  int64_t _startConnectTime;
};

}//namespace base
}//namespace neptune

#endif
