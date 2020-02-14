#include "net.h"

namespace neptune {
namespace base {

ConnectionManager::ConnectionManager(Transport *transport, IPacketStreamer *streamer, IPacketHandler *packetHandler) {
  assert(transport != NULL);
  assert(streamer != NULL);
  _transport = transport;
  _streamer = streamer;
  _packetHandler = packetHandler;
  _queueLimit = 256;
  _queueTimeout = 5000;
  _status = 0;
}

ConnectionManager::~ConnectionManager() {
}

void ConnectionManager::cleanup() {
  _status = 1;
  _mutex.lock();
  NEP_NET_CONN_MAP::iterator it;
  for (it = _connectMap.begin(); it != _connectMap.end(); ++it) {
    _transport->disconnect(it->second);
  }
  _connectMap.clear();
  _mutex.unlock();
}

Connection *ConnectionManager::getConnection(uint64_t serverId) {
  if (_status == 1) {
    return NULL;
  }
  if (serverId == 0) {
    return NULL;
  }

  CThreadGuard guard(&_mutex);

  NEP_NET_CONN_MAP::iterator it ;
  it = _connectMap.find(serverId);
  if (it != _connectMap.end()) {
    return it->second;
  }

  char spec[64];
  sprintf(spec, "tcp:%s", CNetUtil::addrToString(serverId).c_str());
  Connection *conn = _transport->connect(spec, _streamer, true);
  if (!conn) {
  //    LOG(WARN, "���ӵ�������ʧ��: %s", spec);
    return NULL;
  } else {
    conn->setDefaultPacketHandler(_packetHandler);
    conn->setQueueLimit(_queueLimit);
    conn->setQueueTimeout(_queueTimeout);
    _connectMap[serverId] = conn;
  }
  return conn;
}

Connection *ConnectionManager::connect(uint64_t serverId,
                                       IPacketHandler *packetHandler, int queueLimit, int queueTimeout) {
  Connection *conn = getConnection(serverId);
  if (conn != NULL) {
    conn->setDefaultPacketHandler(packetHandler);
    conn->setQueueLimit(queueLimit);
    conn->setQueueTimeout(queueTimeout);
  }
  return conn;
}

void ConnectionManager::disconnect(uint64_t serverId) {
  _mutex.lock();
  NEP_NET_CONN_MAP::iterator it = _connectMap.find(serverId);
  if (it != _connectMap.end()) {
    if (it->second) {
      _transport->disconnect(it->second);
    }
    _connectMap.erase(it);
  }
  _mutex.unlock();
}

void ConnectionManager::setDefaultQueueLimit(uint64_t serverId, int queueLimit) {
  if (serverId) {
    Connection *conn = getConnection(serverId);
    if (conn != NULL) {
      conn->setQueueLimit(queueLimit);
    }
  } else {
    _queueLimit = queueLimit;
  }
}

void ConnectionManager::setDefaultQueueTimeout(uint64_t serverId, int queueTimeout) {
  if (serverId) {
    Connection *conn = getConnection(serverId);
    if (conn != NULL) {
      conn->setQueueTimeout(queueTimeout);
    }
  } else {
    _queueTimeout = queueTimeout;
  }
}

void ConnectionManager::setDefaultPacketHandler(uint64_t serverId, IPacketHandler *packetHandler) {
  if (serverId) {
    Connection *conn = getConnection(serverId);
    if (conn != NULL) {
      conn->setDefaultPacketHandler(packetHandler);
    }
  } else {
    _packetHandler = packetHandler;
  }
}

bool ConnectionManager::sendPacket(uint64_t serverId, Packet *packet, IPacketHandler *packetHandler, void *args, bool noblocking) {
  Connection *conn = getConnection(serverId);
  if (conn) {
    return conn->postPacket(packet, packetHandler, args, noblocking);
  }
  return false;
}

bool ConnectionManager::isAlive(uint64_t serverId) {
  Socket socket;
  char ip[32];
  strcpy(ip, CNetUtil::addrToString((serverId&0xffffffff)).c_str());
  int port = ((serverId >> 32) & 0xffff);
  if (socket.setAddress(ip, port) == false) {
    return false;
  }
  socket.setTimeOption(SO_RCVTIMEO, 1000);
  socket.setTimeOption(SO_SNDTIMEO, 1000);
  if (socket.connect()) {
    return true;
  }
  return false;
}

} //namespace base
} //namespace neptune

