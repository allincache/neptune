#ifndef N_BASE_NET_SOCKET_H_
#define N_BASE_NET_SOCKET_H_
#include <string>

namespace neptune {
namespace base {

class Socket {

 public:
  Socket();

  ~Socket();

  bool setAddress (const char *address, const int port);

  bool connect();

  void close();

  void shutdown();

  bool createUDP();

  void setUp(int socketHandle, struct sockaddr *hostAddress);

  int getSocketHandle();

  IOComponent *getIOComponent();

  void setIOComponent(IOComponent *ioc);

  int write(const void *data, int len);

  int read(void *data, int len);

  bool setKeepAlive(bool on) {
      return setIntOption(SO_KEEPALIVE, on ? 1 : 0);
  }

  bool setReuseAddress(bool on) {
      return setIntOption(SO_REUSEADDR, on ? 1 : 0);
  }

  bool setSoLinger (bool doLinger, int seconds);

  bool setTcpNoDelay(bool noDelay);

  bool setTcpQuickAck(bool quickAck);

  bool setIntOption(int option, int value);

  bool setTimeOption(int option, int milliseconds);

  bool setSoBlocking(bool on);

  bool checkSocketHandle();

  int getSoError();

  std::string getAddr();

  uint64_t getId();
  uint64_t getPeerId();

  int getLocalPort();

  static int getLastError() {
      return errno;
  }

 protected:
  struct sockaddr_in  _address; 
  int _socketHandle;    
  IOComponent *_iocomponent;
  static CThreadMutex _dnsMutex; 
};

}//namespace base
}//namespace neptune

#endif
