#ifndef N_BASE_NET_CONNECTION_H_
#define N_BASE_NET_CONNECTION_H_

#define READ_WRITE_SIZE 8192
#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif

namespace neptune {
namespace base {

class Connection {

 public:
  Connection(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter);

  virtual ~Connection();

  void setServer(bool isServer) {
    _isServer = isServer;
  }

  void setIOComponent(IOComponent *ioc) {
    _iocomponent = ioc;
  }

  IOComponent *getIOComponent() {
    return _iocomponent;
  }

  void setDefaultPacketHandler(IPacketHandler *ph) {
    _defaultPacketHandler = ph;
  }

  bool postPacket(Packet *packet, IPacketHandler *packetHandler = NULL, void *args = NULL, bool noblocking = true);

  bool handlePacket(DataBuffer *input, PacketHeader *header);

  bool checkTimeout(int64_t now);

  virtual bool writeData() = 0;

  virtual bool readData() = 0;

  virtual void setWriteFinishClose(bool v) {
    UNUSED(v);
  }

  void setQueueTimeout(int queueTimeout) {
    _queueTimeout = queueTimeout;
  }

  virtual void clearOutputBuffer() {
    ;
  }

  void setQueueLimit(int limit) {
    _queueLimit = limit;
  }

  bool isConnectState();

  uint64_t getServerId() {
    if (_socket) {
      return _socket->getId();
    }
    return 0;
  }

  uint64_t getPeerId() {
    if (_socket) {
      return _socket->getPeerId();
    }
    return 0;
  }

  int getLocalPort() {
    if (_socket) {
      return _socket->getLocalPort();
    }
    return -1;
  }

 protected:
  void disconnect();

 protected:
  IPacketHandler *_defaultPacketHandler;  
  bool _isServer;                        
  IOComponent *_iocomponent;
  Socket *_socket;                        
  IPacketStreamer *_streamer;             
  IServerAdapter *_serverAdapter;         

  PacketQueue _outputQueue;               
  PacketQueue _inputQueue;                
  PacketQueue _myQueue;                   
  CThreadCond _outputCond;         
  ChannelPool _channelPool;               
  int _queueTimeout;                      
  int _queueTotalSize;                    
  int _queueLimit;                      
};

} //namespace base
} //namespace neptune

#endif
