#include "net.h"

namespace neptune {
namespace base {

ServerSocket::ServerSocket() {
  _backLog = 256;
}

Socket *ServerSocket::accept() {
  Socket *handleSocket = NULL;

  struct sockaddr_in addr;
  int len = sizeof(addr);

  int fd = ::accept(_socketHandle, (struct sockaddr *) & addr, (socklen_t*) & len);

  if (fd >= 0) {
    handleSocket = new Socket();
    handleSocket->setUp(fd, (struct sockaddr *)&addr);
  } else {
      int error = getLastError();
      if (error != EAGAIN) {
          //LOG(ERROR, "%s(%d)", strerror(error), error);
      }
  }

  return handleSocket;
}

bool ServerSocket::listen() {
  if (!checkSocketHandle()) {
      return false;
  }

  setSoLinger(false, 0);
  setReuseAddress(true);
  setIntOption(SO_KEEPALIVE, 1);
  setIntOption(SO_SNDBUF, 640000);
  setIntOption(SO_RCVBUF, 640000);
  setTcpNoDelay(true);

  if (::bind(_socketHandle, (struct sockaddr *)&_address,
    sizeof(_address)) < 0) {
    return false;
  }

  if (::listen(_socketHandle, _backLog) < 0) {
    return false;
  }

  return true;
}

}//namespace base
}//namespace neptune
