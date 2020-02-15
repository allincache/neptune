#include "Process.h"

namespace neptune {
namespace base {
    
int CProcess::startDaemon(const string& pidFile, const string& logFile)
{
  if (getppid() == 1) {
    return 0;
  }

  int pid = fork();
  if (pid < 0) exit(1);
  if (pid > 0) return pid;

  writePidFile(pidFile.c_str());

  int fd =open("/dev/null", 0);
  if (fd != -1) {
    dup2(fd, 0);
    close(fd);
  }
  
  LOGGER.setFileName(logFile.c_str());
          
  return pid;
}

void CProcess::writePidFile(const string& pidFile)
{
  char buf[32];
  int fp = ::open(pidFile.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0600);
  if (fp < 0) exit(1);
  if (lockf(fp, F_TLOCK, 0) < 0) {
    fprintf(stderr, "Can't Open Pid File: %s", pidFile.c_str());
    exit(0);
  }
  sprintf(buf, "%d\n", getpid());
  ssize_t len = strlen(buf);
  ssize_t ret = ::write(fp, buf, len);
  if (ret != len ) {
    fprintf(stderr, "Can't Write Pid File: %s", pidFile.c_str());
    exit(0);
  }
  close(fp);
}
    
int CProcess::existPid(const string& pidFile)
{
  char buf[64], *p;
  int otherpid = 0, fp;
  fp = ::open(pidFile.c_str(), O_RDONLY, 0);
  if (fp >= 0) {
    ::read(fp, buf, 64);
    ::close(fp);
    buf[63] = '\0';
    p = strchr(buf, '\n');
    if (p != NULL)
      *p = '\0';
    otherpid = atoi(buf);
  }
  if (otherpid > 0) {
    if (::kill(otherpid, 0) != 0) {
      otherpid = 0;
    }
  }
  return otherpid;
}

} //namespace base
} //namespace neptune


