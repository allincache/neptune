#ifndef N_BASE_NET_ISERVERADAPTER_H
#define N_BASE_NET_ISERVERADAPTER_H

#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif

namespace neptune {
namespace base {

class IServerAdapter {
  friend class Connection;
  friend class TCPConnection;

 public:
  virtual IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet) = 0;
  virtual bool handleBatchPacket(Connection *connection, PacketQueue &packetQueue) {
    UNUSED(packetQueue);
    UNUSED(connection);
    return false;
  }
  IServerAdapter() {
    _batchPushPacket = false;
  }
  virtual ~IServerAdapter() {}
  void setBatchPushPacket(bool value) {
    _batchPushPacket = value;
  }
 
 private:
  bool _batchPushPacket;       
};

}//namespace base
}//namespace neptune

#endif
