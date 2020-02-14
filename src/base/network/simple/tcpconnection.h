#ifndef N_BASE_NET_TCPCONNECTION_H_
#define N_BASE_NET_TCPCONNECTION_H_

namespace neptune {
namespace base {

class TCPConnection : public Connection {

 public:
  TCPConnection(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter);

  ~TCPConnection();

  bool writeData();

  bool readData();

  void setWriteFinishClose(bool v) {
      _writeFinishClose = v;
  }

  void clearOutputBuffer() {
      _output.clear();
  }

  void clearInputBuffer() {
      _input.clear();
  }

  void setDisconnState();

 private:
  DataBuffer _output;      
  DataBuffer _input;       
  PacketHeader _packetHeader;  
  bool _gotHeader;             
  bool _writeFinishClose;     
};

}//namespace base
}//namespace neptune

#endif
