#include "net.h"

namespace neptune {
namespace base {

IOComponent::IOComponent(Transport *owner, Socket *socket) {
  assert(socket);
  _owner = owner;
  _socket = socket;
  _socket->setIOComponent(this);
  _socketEvent = NULL;
  atomic_set(&_refcount, 0);
  _state = NET_UNCONNECTED; 
  _autoReconn = false; 
  _prev = _next = NULL;
  _lastUseTime = CTimeUtil::getTime();
  _inUsed = false;
}

IOComponent::~IOComponent() {
  if (_socket) {
    _socket->close();
    delete _socket;
    _socket = NULL;
  }
}

/**
 * owner
 */
Transport *IOComponent::getOwner()
{
  return _owner;
}

}//namespace base
}//namespace neptune
