#ifndef N_BASE_NET_TCPACCEPTOR_H_
#define N_BASE_NET_TCPACCEPTOR_H_

namespace neptune {
namespace base {

class TCPAcceptor : public IOComponent {

 public:
  TCPAcceptor(Transport *owner, Socket *socket,
              IPacketStreamer *streamer, IServerAdapter *serverAdapter);

  bool init(bool isServer = false);

  bool handleReadEvent();

  bool handleWriteEvent() {
    return true;
  }

  void checkTimeout(int64_t now);

 private:
  IPacketStreamer *_streamer;      
  IServerAdapter  *_serverAdapter; 
};

}//namespace base
}//namespace neptune

#endif
