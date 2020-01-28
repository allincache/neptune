#ifndef N_DFS_UTIL_FILE_QUEUE_H
#define N_DFS_UTIL_FILE_QUEUE_H

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "base/log/Log.h"
#include "base/thread/ThreadMutex.h"
#include "base/fs/FileUtil.h"

namespace neptune {
namespace dfs {
    
#define TBFQ_MAX_FILE_SIZE (1024*1024*16)    //16M
#define TBFQ_MAX_THREAD_COUNT 60
#define TBFQ_FILE_QUEUE_FLAG  0x31765166     // fQv1

typedef struct file_unsettle {
  uint32_t seqno;
  int offset;
} file_unsettle;

typedef struct queue_item {
  file_unsettle pos;
  int flag;
  int len;
  char data[0];
} queue_item;

typedef struct qinfo_head {
  uint32_t read_seqno;    
  int read_offset;        
  uint32_t write_seqno;        
  int write_filesize;     
  int queue_size;         
  int exit_status;        
  int reserve[2];            
  file_unsettle pos[TBFQ_MAX_THREAD_COUNT];
} qinfo_head;

class CFileUtil;

class CFileQueue {
 public:
  CFileQueue(char *rootPath, char *queueName, int maxFileSize = TBFQ_MAX_FILE_SIZE);
  ~CFileQueue(void);
  int push(void *data, int len);
  queue_item *pop(uint32_t index = 0);
  int clear();
  int isEmpty();
  void finish(uint32_t index = 0);
  void backup(uint32_t index = 0);

 private:
  int m_readFd;
  int m_writeFd;
  int m_infoFd;
  qinfo_head m_head;
  char *m_queuePath;
  int m_maxFileSize;

 private:
  inline int openWriteFile();  
  inline int openReadFile(); 
  inline int deleteReadFile();            
  inline int writeHead();
  void recoverRecord();               
};

}//namespace dfs
}//namespace neptune

#endif
