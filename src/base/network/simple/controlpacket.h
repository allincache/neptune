#ifndef N_BASE_NET_CONTROL_PACKET_H_
#define N_BASE_NET_CONTROL_PACKET_H_
#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif

namespace neptune {
namespace base {

class ControlPacket : public Packet {
 public:
  enum {
      CMD_BAD_PACKET = 1,
      CMD_TIMEOUT_PACKET,
      CMD_DISCONN_PACKET
  };

  static ControlPacket BadPacket;
  static ControlPacket TimeoutPacket;
  static ControlPacket DisconnPacket;

public:
  ControlPacket(int c) : _command(c) {}

  bool isRegularPacket() {
      return false;
  }

  void free() {}

  void countDataLen() {}

  bool encode(DataBuffer *output) {
    UNUSED(output);
      return false;
  }

  bool decode(DataBuffer *input, PacketHeader *header) {
    UNUSED(input);
    UNUSED(header);
      return false;
  }

  int getCommand() {
    return _command;
  }

 private:
  int _command;
};

} //namespace base
} //namespace neptune

#endif
