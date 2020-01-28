#include "net.h"

namespace neptune {
namespace base {

Packet::Packet() {
  _next = NULL;
  _channel = NULL;
  _expireTime = 0;
  memset(&_packetHeader, 0, sizeof(PacketHeader));
}

Packet::~Packet() {
}

void Packet::setChannel(Channel *channel) {
  if (channel) {
    _channel = channel;
    _packetHeader._chid = channel->getId();
  }
}

void Packet::setExpireTime(int milliseconds) {
  if (milliseconds == 0) {
    milliseconds = 1000*86400;
  }
  _expireTime = CTimeUtil::getTime() + static_cast<int64_t>(milliseconds) * static_cast<int64_t>(1000);
}

}//namespace base
}//namespace neptune
