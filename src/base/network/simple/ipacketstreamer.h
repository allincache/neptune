#ifndef N_BASE_NET_IPACKETSTREAMER_H_
#define N_BASE_NET_IPACKETSTREAMER_H_

namespace neptune {
namespace base {

class IPacketStreamer {

 public:
  IPacketStreamer() {
      _factory = NULL;
      _existPacketHeader = true;
  }

  IPacketStreamer(IPacketFactory *factory) {
      _factory = factory;
      _existPacketHeader = true;
  }

  virtual ~IPacketStreamer() {}

  virtual bool getPacketInfo(DataBuffer *input, PacketHeader *header, bool *broken) = 0;

  virtual Packet *decode(DataBuffer *input, PacketHeader *header) = 0;

  virtual bool encode(Packet *packet, DataBuffer *output) = 0;

  bool existPacketHeader() {
      return _existPacketHeader;
  }

 protected:
  IPacketFactory *_factory;   
  bool _existPacketHeader;    
};

}//namespace base
}//namespace neptune

#endif
