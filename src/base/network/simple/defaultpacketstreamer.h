#ifndef N_BASE_NET_DEFAULT_PACKET_STREAMER_H_
#define N_BASE_NET_DEFAULT_PACKET_STREAMER_H_

namespace neptune {
namespace base {

class DefaultPacketStreamer : public IPacketStreamer {

 public:
  DefaultPacketStreamer();

  DefaultPacketStreamer(IPacketFactory *factory);

  ~DefaultPacketStreamer();

  void setPacketFactory(IPacketFactory *factory);

  bool getPacketInfo(DataBuffer *input, PacketHeader *header, bool *broken);

  Packet *decode(DataBuffer *input, PacketHeader *header);

  bool encode(Packet *packet, DataBuffer *output);

  static void setPacketFlag(int flag);

 public:
  static int _nPacketFlag;
};

} //namespace base
} //namespace neptune

#endif
