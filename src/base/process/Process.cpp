#include "Process.h"

namespace neptune {
namespace base {
    
int CProcess::startDaemon(const char *szPidFile, const char *szLogFile)
{
  if (getppid() == 1) {
    return 0;
  }

  int pid = fork();
  if (pid < 0) exit(1);
  if (pid > 0) return pid;

  writePidFile(szPidFile);

  int fd =open("/dev/null", 0);
  if (fd != -1) {
    dup2(fd, 0);
    close(fd);
  }
  
  LOGGER.setFileName(szLogFile);
          
  return pid;
}

void CProcess::writePidFile(const char *szPidFile)
{
  char            str[32];
  int lfp = open(szPidFile, O_WRONLY|O_CREAT|O_TRUNC, 0600);
  if (lfp < 0) exit(1);
  if (lockf(lfp, F_TLOCK, 0) < 0) {
    fprintf(stderr, "Can't Open Pid File: %s", szPidFile);
    exit(0);
  }
  sprintf(str, "%d\n", getpid());
  ssize_t len = strlen(str);
  ssize_t ret = write(lfp, str, len);
  if (ret != len ) {
    fprintf(stderr, "Can't Write Pid File: %s", szPidFile);
    exit(0);
  }
  close(lfp);
}
    
int CProcess::existPid(const char *szPidFile)
{
  char buffer[64], *p;
  int otherpid = 0, lfp;
  lfp = open(szPidFile, O_RDONLY, 0);
  if (lfp >= 0) {
    read(lfp, buffer, 64);
    close(lfp);
    buffer[63] = '\0';
    p = strchr(buffer, '\n');
    if (p != NULL)
      *p = '\0';
    otherpid = atoi(buffer);
  }
  if (otherpid > 0) {
    if (kill(otherpid, 0) != 0) {
      otherpid = 0;
    }
  }
  return otherpid;
}

} //namespace base
} //namespace neptune


