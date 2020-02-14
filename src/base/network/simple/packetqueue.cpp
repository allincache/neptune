#include "net.h"

namespace neptune {
namespace base {

PacketQueue::PacketQueue() {
  _head = NULL;
  _tail = NULL;
  _size = 0;
}

PacketQueue::~PacketQueue() {
  clear();
}

Packet *PacketQueue::pop() {
  if (_head == NULL) {
    return NULL;
  }
  Packet *packet = _head;
  _head = _head->_next;
  if (_head == NULL) {
    _tail = NULL;
  }
  _size --;
  return packet;
}

void PacketQueue::clear() {
  if (_head == NULL) {
    return;
  }
  while (_head != NULL) {
    Packet *packet = _head;
    _head = packet->_next;
    packet->free();
  }
  _head = _tail = NULL;
  _size = 0;
}

void PacketQueue::push(Packet *packet) {
  if (packet == NULL) {
    //LOG(INFO, "packet is null.");
    return;
  }
  packet->_next = NULL;
  if (_tail == NULL) {
    _head = packet;
  } else {
    _tail->_next = packet;
  }
  _tail = packet;
  _size++;
}

int PacketQueue::size() {
  return _size;
}

bool PacketQueue::empty() {
  return (_size == 0);
}

void PacketQueue::moveTo(PacketQueue *destQueue) {
  if (_head == NULL) { 
    return;
  }
  if (destQueue->_tail == NULL) {
    destQueue->_head = _head;
  } else {
    destQueue->_tail->_next = _head;
  }
  destQueue->_tail = _tail;
  destQueue->_size += _size;
  _head = _tail = NULL;
  _size = 0;
}

Packet *PacketQueue::getTimeoutList(int64_t now) {
  Packet *list, *tail;
  list = tail = NULL;
  while (_head != NULL) {
    int64_t t = _head->getExpireTime();
    if (t == 0 || t >= now) break;
    if (tail == NULL) {
      list = _head;
    } else {
      tail->_next = _head;
    }
    tail = _head;

    _head = _head->_next;
    if (_head == NULL) {
      _tail = NULL;
    }
    _size --;
  }
  if (tail) {
    tail->_next = NULL;
  }
  return list;
}

Packet *PacketQueue::getPacketList() {
  return _head;
}

}//namespace base
}//namespace neptune

