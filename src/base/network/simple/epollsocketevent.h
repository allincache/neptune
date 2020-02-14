#ifndef N_BASE_NET_EPOLLSOCKETEVENT_H_
#define N_BASE_NET_EPOLLSOCKETEVENT_H_

namespace neptune {
namespace base {

class EPollSocketEvent : public SocketEvent {

 public:
  EPollSocketEvent();

  ~EPollSocketEvent();

  bool addEvent(Socket *socket, bool enableRead, bool enableWrite);

  bool setEvent(Socket *socket, bool enableRead, bool enableWrite);

  bool removeEvent(Socket *socket);

  int getEvents(int timeout, IOEvent *events, int cnt);

 private:
  int _iepfd;
//    CThreadMutex _mutex;
};

} //namespace base
} //namespace neptune

#endif
