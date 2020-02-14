#ifndef N_BASE_NET_TRANSPORT_H_
#define N_BASE_NET_TRANSPORT_H_

#include <sys/ptrace.h>

namespace neptune {
namespace base {

class Transport : public Runnable {

 public:
  Transport();
  ~Transport();

  bool start();
  bool stop();
  bool wait();
  void run(CThread *thread, void *arg);

  IOComponent *listen(const char *spec, IPacketStreamer *streamer, IServerAdapter *serverAdapter);
  Connection *connect(const char *spec, IPacketStreamer *streamer, bool autoReconn = false);

  bool disconnect(Connection *conn);
  void addComponent(IOComponent *ioc, bool readOn, bool writeOn);
  void removeComponent(IOComponent *ioc);
  bool* getStop();

 private:
  int parseAddr(char *src, char **args, int cnt);
  void eventLoop(SocketEvent *socketEvent);
  void timeoutLoop();
  void destroy();

 private:
  EPollSocketEvent _socketEvent;      
  CThread _readWriteThread;    
  CThread _timeoutThread;      
  bool _stop;                         

  IOComponent *_delListHead, *_delListTail;  
  IOComponent *_iocListHead, *_iocListTail;   
  bool _iocListChanged;                       
  int _iocListCount;
  CThreadMutex _iocsMutex;
};

}//namespace base
}//namespace neptune

#endif
