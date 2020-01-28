#ifndef N_BASE_NET_HTTP_PACKET_STREAMER_H
#define N_BASE_NET_HTTP_PACKET_STREAMER_H

namespace neptune {
namespace base {

class HttpPacketStreamer : public DefaultPacketStreamer {
 public:
  HttpPacketStreamer();
  HttpPacketStreamer(IPacketFactory *factory);
  bool getPacketInfo(DataBuffer *input, PacketHeader *header, bool *broken);
  void setHttpPacketCode(int code) {
      _httpPacketCode = code;
  }
 private:
  int _httpPacketCode;
};

class DefaultHttpPacketFactory : public IPacketFactory {
 public:
  Packet *createPacket(int pcode) {
    if (pcode == 1) {
      return new HttpRequestPacket();
    } else {
      return new HttpResponsePacket();
    }
  }
};

}//namespace base
}//namespace neptune

#endif

