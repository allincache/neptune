#include "net.h"

namespace neptune {
namespace base {

PacketQueueThread::PacketQueueThread() : CDefaultRunnable() {
  _stop = 0;
  _waitFinish = false;
  _handler = NULL;
  _args = NULL;
  _waitTime = 0;
  _waiting = false;
  _speed_t2 = _speed_t1 = CTimeUtil::getTime();
  _overage = 0;
}

PacketQueueThread::PacketQueueThread(int threadCount, IPacketQueueHandler *handler, void *args)
      : CDefaultRunnable(threadCount) {
  _stop = 0;
  _waitFinish = false;
  _handler = handler;
  _args = args;
  _waitTime = 0;
  _waiting = false;
  _speed_t2 = _speed_t1 = CTimeUtil::getTime();
  _overage = 0;
}

PacketQueueThread::~PacketQueueThread() {
  stop();
}

void PacketQueueThread::setThreadParameter(int threadCount, IPacketQueueHandler *handler, void *args) {
  setThreadCount(threadCount);
  _handler = handler;
  _args = args;
}

void PacketQueueThread::stop(bool waitFinish) {
  _cond.lock();
  _stop = true;
  _waitFinish = waitFinish;
  _cond.broadcast();
  _cond.unlock();
}

// push
// block==true, this thread can wait util _queue.size less than maxQueueLen
// otherwise, return false directly, client must be free this packet.
bool PacketQueueThread::push(Packet *packet, int maxQueueLen, bool block) {
  // if queue stoped or not started yet, free packet
  if (_stop || _thread == NULL) {
    delete packet;
    return true;
  }
  // check max length of this queue
  if (maxQueueLen>0 && _queue._size >= maxQueueLen) {
      _pushcond.lock();
      _waiting = true;
      while (_stop == false && _queue.size() >= maxQueueLen && block) {
          _pushcond.wait(1000);
      }
      _waiting = false;
      if (_queue.size() >= maxQueueLen && !block)
      {
          _pushcond.unlock();
          return false;
      }
      _pushcond.unlock();
      
      if (_stop) {
          delete packet;
          return true;
      }
  }
  _cond.lock();
  _queue.push(packet);
  _cond.unlock();
  _cond.signal();
  return true;
}

// pushQueue
void PacketQueueThread::pushQueue(PacketQueue &packetQueue, int maxQueueLen) {
  if (_stop) {
      return;
  }
  if (maxQueueLen>0 && _queue._size >= maxQueueLen) {
      _pushcond.lock();
      _waiting = true;
      while (_stop == false && _queue.size() >= maxQueueLen) {
          _pushcond.wait(1000);
      }
      _waiting = false;
      _pushcond.unlock();
      if (_stop) {
          return;
      }
  }
  _cond.lock();
  packetQueue.moveTo(&_queue);
  _cond.unlock();
  _cond.signal();
}

void PacketQueueThread::run(CThread *thread, void *arg) {
  Packet *packet = NULL;
  while (!_stop) {
      _cond.lock();
      while (!_stop && _queue.size() == 0) {
          _cond.wait();
      }
      if (_stop) {
          _cond.unlock();
          break;
      }
      if (_waitTime>0) checkSendSpeed();
      packet = _queue.pop();
      _cond.unlock();
      if (_waiting) {
          _pushcond.lock();
          _pushcond.signal();
          _pushcond.unlock();
      }
      if (packet == NULL) continue;
      bool ret = true;
      if (_handler) {
          ret = _handler->handlePacketQueue(packet, _args);
      }
      if (ret) delete packet;
  }
  if (_waitFinish) { 
    bool ret = true;
      _cond.lock();
      while (_queue.size() > 0) {
          packet = _queue.pop();
          _cond.unlock();
          ret = true;
          if (_handler) {
              ret = _handler->handlePacketQueue(packet, _args);
          }
          if (ret) delete packet;

          _cond.lock();
      }
      _cond.unlock();
  } else {   
      _cond.lock();
      while (_queue.size() > 0) {
          delete _queue.pop();
      }
      _cond.unlock();
  }
}

void PacketQueueThread::setStatSpeed() {
}

void PacketQueueThread::setWaitTime(int t) {
  _waitTime = t;
  _speed_t2 = _speed_t1 = CTimeUtil::getTime();
  _overage = 0;
}

void PacketQueueThread::checkSendSpeed() {
  if (_waitTime > _overage) {
    usleep(_waitTime - _overage);
  }
  _speed_t2 = CTimeUtil::getTime();
  _overage += (_speed_t2-_speed_t1) - _waitTime;
  if (_overage > (_waitTime<<4)) _overage = 0;
  _speed_t1 = _speed_t2;
}

}//namespace base
}//namespace neptune

