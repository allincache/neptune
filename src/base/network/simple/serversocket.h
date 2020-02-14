#ifndef N_BASE_NET_SERVERSOCKET_H_
#define N_BASE_NET_SERVERSOCKET_H_

namespace neptune {
namespace base {

class ServerSocket : public Socket {

 public:
  ServerSocket();
  Socket *accept();
  bool listen();

 private:
  int _backLog; // backlog
};

}//namespace base
}//namespace neptune

#endif
