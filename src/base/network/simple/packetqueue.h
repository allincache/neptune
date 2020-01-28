#ifndef N_BASE_NET_PACKET_QUEUE_H_
#define N_BASE_NET_PACKET_QUEUE_H_

namespace neptune {
namespace base {

class PacketQueue {
  friend class PacketQueueThread;
 public:
  PacketQueue();

  ~PacketQueue();

  Packet *pop();

  void clear();

  void push(Packet *packet);

  int size();

  bool empty();

  void moveTo(PacketQueue *destQueue);

  Packet *getTimeoutList(int64_t now);

  Packet *getPacketList();

  Packet *head()
  {
    return _head;
  }

  Packet* tail()
  {
    return _tail;
  }
protected:
  Packet *_head;  
  Packet *_tail;  
  int _size;      
};

}//namespace base
}//namespace neptune

#endif

