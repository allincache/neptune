#ifndef N_BASE_NET_SOCKETEVENT_H_
#define N_BASE_NET_SOCKETEVENT_H_

namespace neptune {
namespace base {

#define MAX_SOCKET_EVENTS 256

class IOEvent {

 public:
  bool _readOccurred;  
  bool _writeOccurred;  
  bool _errorOccurred;  
  IOComponent *_ioc;  
};

class SocketEvent {

 public:
  SocketEvent();
  virtual ~SocketEvent();
  virtual bool addEvent(Socket *socket, bool enableRead, bool enableWrite) = 0;
  virtual bool setEvent(Socket *socket, bool enableRead, bool enableWrite) = 0;
  virtual bool removeEvent(Socket *socket) = 0;
  virtual int getEvents(int timeout, IOEvent *events, int cnt) = 0;
};

}//namespace base
}//namespace neptune

#endif
