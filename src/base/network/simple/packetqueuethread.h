#ifndef N_BASE_NET_PACKET_QUEUE_THREAD_H
#define N_BASE_NET_PACKET_QUEUE_THREAD_H

namespace neptune {
namespace base {

class IPacketQueueHandler {
public:
  virtual ~IPacketQueueHandler() {}
  virtual bool handlePacketQueue(Packet *packet, void *args) = 0;
};

class PacketQueueThread : public CDefaultRunnable {
 public:
  PacketQueueThread();

  PacketQueueThread(int threadCount, IPacketQueueHandler *handler, void *args);

  ~PacketQueueThread();

  void setThreadParameter(int threadCount, IPacketQueueHandler *handler, void *args);

  // stop
  void stop(bool waitFinish = false);

  // push
  bool push(Packet *packet, int maxQueueLen = 0, bool block = true);

  // pushQueue
  void pushQueue(PacketQueue &packetQueue, int maxQueueLen = 0);

  void run(CThread *thread, void *arg);

  void setStatSpeed();

  void setWaitTime(int t);

  Packet *head()
  {
    return _queue.head();
  }
  Packet *tail()
  {
    return _queue.tail();
  }
  size_t size()
  {
    return _queue.size();
  }
 private:
  //void PacketQueueThread::checkSendSpeed()
  void checkSendSpeed();

 private:
  PacketQueue _queue;
  IPacketQueueHandler *_handler;
  CThreadCond _cond;
  CThreadCond _pushcond;
  void *_args;
  bool _waitFinish;       
  int _waitTime;
  int64_t _speed_t1;
  int64_t _speed_t2;
  int64_t _overage;
  bool _waiting;
};

}//namespace base
}//namespace neptune

#endif

