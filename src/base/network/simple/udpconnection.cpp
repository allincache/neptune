#include "net.h"

namespace neptune {
namespace base {

UDPConnection::UDPConnection(Socket *socket, IPacketStreamer *streamer,
                             IServerAdapter *serverAdapter) : Connection(socket, streamer, serverAdapter) {}

UDPConnection::~UDPConnection() {}

bool UDPConnection::writeData() {
  return true;
}

bool UDPConnection::readData() {
  return true;
}

}//namespace base
}//namespace neptune
