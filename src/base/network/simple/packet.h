#ifndef N_BASE_NET_PACKET_H_
#define N_BASE_NET_PACKET_H_

namespace neptune {
namespace base {

#define NET_PACKET_FLAG 0x416e4574

class PacketHeader {
 public:
  uint32_t _chid;         
  int _pcode;             
  int _dataLen;          
};

class Packet {
  friend class PacketQueue;

 public:
  Packet();

  virtual ~Packet();

  void setChannelId(uint32_t chid) {
    _packetHeader._chid = chid;
  }

  uint32_t getChannelId() const {
    return _packetHeader._chid;
  }

  void setPCode(int pcode) {
    _packetHeader._pcode = pcode;
  }

  int getPCode() const {
    return _packetHeader._pcode;
  }

  PacketHeader *getPacketHeader() {
    return &_packetHeader;
  }

  void setPacketHeader(PacketHeader *header) {
    if (header) {
      memcpy(&_packetHeader, header, sizeof(PacketHeader));
    }
  }

  virtual void free() {
    delete this;
  }

  virtual bool isRegularPacket() {
    return true;
  }

  virtual bool encode(DataBuffer *output) = 0;

  virtual bool decode(DataBuffer *input, PacketHeader *header) = 0;

  int64_t getExpireTime() const {
    return _expireTime;
  }

  void setExpireTime(int milliseconds);

  void setChannel(Channel *channel);

  Channel *getChannel() const {
    return _channel;
  }

  Packet *getNext() const {
    return _next;
  }

 protected:
  PacketHeader _packetHeader; 
  int64_t _expireTime;        
  Channel *_channel;

  Packet *_next;              
};

}//namespace base
}//namespace neptune

#endif
