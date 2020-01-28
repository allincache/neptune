#ifndef N_BASE_NET_IPACKET_FACTORY_H_
#define N_BASE_NET_IPACKET_FACTORY_H_

namespace neptune {
namespace base {

class IPacketFactory {
 public:
  virtual ~IPacketFactory() {};
  virtual Packet *createPacket(int pcode) = 0;
};

}//namespace base
}//namespace neptune

#endif
