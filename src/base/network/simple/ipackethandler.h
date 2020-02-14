#ifndef N_BASE_NET_IPACKETHANDLER_H_
#define N_BASE_NET_IPACKETHANDLER_H_

namespace neptune {
namespace base {

class IPacketHandler {
 public:
  enum HPRetCode {
    KEEP_CHANNEL  = 0,
    CLOSE_CHANNEL = 1,
    FREE_CHANNEL  = 2
  };

  virtual ~IPacketHandler() {}
  virtual HPRetCode handlePacket(Packet *packet, void *args) = 0;
};

}//namespace base
}//namespace neptune

#endif
