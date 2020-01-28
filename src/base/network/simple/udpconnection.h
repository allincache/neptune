#ifndef N_BASE_NET_UDPCONNECTION_H_
#define N_BASE_NET_UDPCONNECTION_H_

namespace neptune {
namespace base {

class UDPConnection : public Connection {
  UDPConnection(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter);
  ~UDPConnection();

  bool writeData();
  bool readData();

};

}//namespace base
}//namespace neptune

#endif
