#include "net.h"

namespace neptune {
namespace base {

TCPComponent::TCPComponent(Transport *owner, Socket *socket,
                           IPacketStreamer *streamer, IServerAdapter *serverAdapter) : IOComponent(owner, socket) {
  _connection = new TCPConnection(socket, streamer, serverAdapter);
  _connection->setIOComponent(this);
  _startConnectTime = 0;
  _isServer = false;
}

TCPComponent::~TCPComponent() {
  if (_connection) {
    _connection->setIOComponent(NULL);
    delete _connection;
    _connection = NULL;
  }
}

bool TCPComponent::init(bool isServer) {
  _socket->setSoBlocking(false);
  _socket->setSoLinger(false, 0);
  _socket->setReuseAddress(true);
  _socket->setIntOption(SO_KEEPALIVE, 1);
  _socket->setIntOption(SO_SNDBUF, 640000);
  _socket->setIntOption(SO_RCVBUF, 640000);
  if (!isServer) {
    if (!socketConnect() && _autoReconn == false) {
      return false;
    }
  } else {
    _state = NET_CONNECTED;
  }
  _connection->setServer(isServer);
  _isServer = isServer;
  return true;
}

bool TCPComponent::socketConnect() {
  if (_state == NET_CONNECTED || _state == NET_CONNECTING) {
    return true;
  }
  _socket->setSoBlocking(false);
  if (_socket->connect()) {
    if (_socketEvent) {
      _socketEvent->addEvent(_socket, true, true);
    }
    _state = NET_CONNECTED; 
    _startConnectTime = CTimeUtil::getTime();
  } else {
    int error = Socket::getLastError();
    if (error == EINPROGRESS || error == EWOULDBLOCK) {
      _state = NET_CONNECTING;
      if (_socketEvent) {
        _socketEvent->addEvent(_socket, true, true);
      }
    } else {
      _socket->close();
      _state = NET_CLOSED;
      //LOG(ERROR, "���ӵ� %s ʧ��, %s(%d)", _socket->getAddr().c_str(), strerror(error), error);
      return false;
    }
  }
  return true;
}

void TCPComponent::close() {
  if (_socket) {
    if (_socketEvent) {
      _socketEvent->removeEvent(_socket);
    }
    if (_connection && isConnectState()) {
      _connection->setDisconnState();
    }
    _socket->close();
    if (_connection) {
      _connection->clearInputBuffer(); // clear input buffer after socket closed
    }
    _state = NET_CLOSED;
  }
}

bool TCPComponent::handleWriteEvent() {
  _lastUseTime = CTimeUtil::getTime();
  bool rc = true;
  if (_state == NET_CONNECTED) {
    rc = _connection->writeData();
  } else if (_state == NET_CONNECTING) {
    int error = _socket->getSoError();
    if (error == 0) {
      enableWrite(true);
      _connection->clearOutputBuffer();
      _state = NET_CONNECTED;
    } else {
      //LOG(ERROR, "���ӵ� %s ʧ��: %s(%d)", _socket->getAddr().c_str(), strerror(error), error);
      if (_socketEvent) {
        _socketEvent->removeEvent(_socket);
      }
      _socket->close();
      _state = NET_CLOSED;
    }
  }
  return rc;
}

bool TCPComponent::handleReadEvent() {
  _lastUseTime = CTimeUtil::getTime();
  bool rc = false;
  if (_state == NET_CONNECTED) {
    rc = _connection->readData();
  }
  return rc;
}

void TCPComponent::checkTimeout(int64_t now) {
  if (_state == NET_CONNECTING) {
    if (_startConnectTime > 0 && _startConnectTime < (now - static_cast<int64_t>(2000000))) { // ���ӳ�ʱ 2 ��
      _state = NET_CLOSED;
      //LOG(ERROR, "���ӵ� %s ��ʱ.", _socket->getAddr().c_str());
      _socket->shutdown();
    }
  } else if (_state == NET_CONNECTED && _isServer == true && _autoReconn == false) { // ���ӵ�ʱ��, ֻ���ڷ�������
    int64_t idle = now - _lastUseTime;
    if (idle > static_cast<int64_t>(900000000)) { // ����15min�Ͽ�
      _state = NET_CLOSED;
      //LOG(INFO, "%s ������: %d (s) ���Ͽ�.", _socket->getAddr().c_str(), (idle/static_cast<int64_t>(1000000)));
      _socket->shutdown();
    }
  }
  _connection->checkTimeout(now);
}

}//namespace base
}//namespace neptune
