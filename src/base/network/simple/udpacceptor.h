#ifndef N_BASE_NET_TCPACCEPTOR_H_
#define N_BASE_NET_TCPACCEPTOR_H_

namespace neptune {
namespace base {

class UDPAcceptor : public UDPComponent {

 public:
  UDPAcceptor(Transport *owner, char *spec, IPacketStreamer *streamer, IServerAdapter *serverAdapter);
  bool handleReadEvent();
  bool handleWriteEvent() {
    return false;
  }
};

}//namespace base
}//namespace neptune

#endif
