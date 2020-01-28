#ifndef N_DFS_UTIL_SERVICE_H
#define N_DFS_UTIL_SERVICE_H

#include <string>
#include "base/network/simple/net.h"
#include "base/concurrent/Mutex.h"
#include "base/common/Monitor.h"

using namespace neptune::base;

namespace neptune {
namespace dfs {

class Service
{
 public:
  Service();
  virtual ~Service();

  int main(int argc, char* argv[]);
  static Service* instance();    
  bool service() const;
  int handleInterrupt(int sig);

 protected:
  void stop();
  int shutdown();
  virtual int run( int argc , char*argv[], const std::string& config, std::string& errMsg)=0;
  virtual int interruptCallback( int sig );
  virtual bool destroy()=0;
  virtual void help();
  virtual void version();
  void enableInterrupt();
  void disableInterrupt();

 private:
  int runDaemon( int argc ,char* argv[] );
  int waitForShutdown();
  void configureDaemon( bool changeDir , bool closeFile);
  int start(int argc , char* argv[] );
  virtual int  initialize(); 

 private:
  bool _nohup;
  bool _service;
  bool _changeDir;
  bool _closeFiles;
  bool _destroyed;
  bool _chlidStop;
  std::string _pidFile;
  std::string _configFile;
  std::string _cmd;
  std::string _chstdOut;
  std::string _chstdErr;
  std::string _noinit;
  Monitor<Mutex> _monitor;
  static Service* _instance;
}; //end Servcie

} //namespace dfs
} //namespace neptune

#endif //N_DFS_UTIL_SERVICE_H
