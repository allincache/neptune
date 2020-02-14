#ifndef N_BASE_NET_IOCOMPONENT_H_
#define N_BASE_NET_IOCOMPONENT_H_

namespace neptune {
namespace base {

#define NET_MAX_TIME (1ll<<62)

class IOComponent {
  friend class Transport;

public:
  enum {
    NET_CONNECTING = 1,
    NET_CONNECTED,
    NET_CLOSED,
    NET_UNCONNECTED
  };

 public:
  IOComponent(Transport *owner, Socket *socket);
  virtual ~IOComponent();

  virtual bool init(bool isServer = false) = 0;

  virtual void close() {}

  virtual bool handleWriteEvent() = 0;

  virtual bool handleReadEvent() = 0;

  virtual void checkTimeout(int64_t now) = 0;

  Socket *getSocket() {
    return _socket;
  }

  void setSocketEvent(SocketEvent *socketEvent) {
    _socketEvent = socketEvent;
  }

  void enableWrite(bool writeOn) {
    if (_socketEvent) {
      _socketEvent->setEvent(_socket, true, writeOn);
    }
  }

  int addRef() {
    return atomic_add_return(1, &_refcount);
  }

  void subRef() {
    atomic_dec(&_refcount);
  }

  int getRef() {
    return atomic_read(&_refcount);
  }

  bool isConnectState() {
    return (_state == NET_CONNECTED || _state == NET_CONNECTING);
  }

  int getState() {
    return _state;
  }

  void setAutoReconn(bool on) {
    _autoReconn = on;
  }

  bool isAutoReconn() {
    return (_autoReconn && !_isServer);
  }

  bool isUsed() {
    return _inUsed;
  }

  void setUsed(bool b) {
    _inUsed = b;
  }

  int64_t getLastUseTime() {
    return _lastUseTime;
  }
  
  Transport *getOwner();

 protected:
  Transport *_owner;
  Socket *_socket;    
  SocketEvent *_socketEvent;
  int _state;         
  atomic_t _refcount; 
  bool _autoReconn;   
  bool _isServer;     
  bool _inUsed;       
  int64_t _lastUseTime;   

 private:
  IOComponent *_prev; 
  IOComponent *_next; 
};

}//namespace base
}//namespace neptune

#endif
