#ifndef NEP_BASE_LOG_LOG_H
#define NEP_BASE_LOG_LOG_H

#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <deque>
#include <string>
#include <pthread.h>
#include <sys/time.h>

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_USER_ERROR  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_TRACE 4
#define LOG_LEVEL_DEBUG 5
#define LOG_LEVEL(level) LOG_LEVEL_##level, __FILE__, __LINE__, __FUNCTION__, pthread_self()
#define LOG_NUM_LEVEL(level) level, __FILE__, __LINE__, __FUNCTION__, pthread_self()
#define LOGGER CLogger::getLogger()
#define LOG_PRINT(level, ...) LOGGER.logMessage(LOG_LEVEL(level), __VA_ARGS__)
#define LOG_BASE(level, ...) (LOG_LEVEL_##level>LOGGER._level) ? (void)0 : LOG_PRINT(level, __VA_ARGS__)
#define LOG(level, _fmt_, args...) ((LOG_LEVEL_##level>LOGGER._level) ? (void)0 : LOG_BASE(level,_fmt_, ##args))
#define LOG_US(level, _fmt_, args...) \
  ((LOG_LEVEL_##level>LOGGER._level) ? (void)0 : LOG_BASE(level, "[%ld][%ld][%ld] " _fmt_, \
  pthread_self(), CLogger::get_cur_tv().tv_sec, \
  CLogger::get_cur_tv().tv_usec, ##args))

namespace neptune {
namespace base {

using std::deque;
using std::string;

class CLogger {

 public:
  static const mode_t LOG_FILE_MODE = 0644;

  CLogger();
  ~CLogger();

  void rotateLog(const char *filename, const char *fmt = NULL);
  void logMessage(int level, const char *file, int line, const char *function, pthread_t tid, const char *fmt, ...) __attribute__ ((format (printf, 7, 8)));
  void setLogLevel(const char *level, const char *wf_level = NULL);
  void setFileName(const char *filename, bool flag = false, bool open_wf = false);
  void checkFile();
  void setCheck(int v) {_check = v;}
  
  void setMaxFileSize( int64_t maxFileSize=0x40000000);
  void setMaxFileIndex( int maxFileIndex= 0x0F);

  static inline struct timeval get_cur_tv()
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
  };

  static CLogger& getLogger();

 private:
  int _fd;
  int _wf_fd;
  char *_name;
  int _check;
  size_t _maxFileIndex;
  int64_t _maxFileSize;
  bool _flag;
  bool _wf_flag;

 public:
  int _level;
  int _wf_level;

 private:
  std::deque<std::string> _fileList;
  std::deque<std::string> _wf_file_list;
  static const char *const _errstr[];
  pthread_mutex_t _fileSizeMutex;
  pthread_mutex_t _fileIndexMutex;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_LOG_LOG_H
