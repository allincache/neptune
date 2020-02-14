#ifndef NEP_BASE_PROCESS_P_H
#define NEP_BASE_PROCESS_P_H

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "base/log/Log.h"

namespace neptune {
namespace base {
    
class CProcess {

 public:
  static int startDaemon(const char *szPidFile, const char *szLogFile);
  static int existPid(const char *szPidFile);
  static void writePidFile(const char *szPidFile);
  
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_PROCESS_P_H
