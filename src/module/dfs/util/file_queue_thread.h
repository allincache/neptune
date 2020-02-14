#ifndef N_DFS_UTIL_FILE_QUEUE_THREAD_H_
#define N_DFS_UTIL_FILE_QUEUE_THREAD_H_

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include "dfs/util/dfs.h"
#include "base/concurrent/Mutex.h"
#include "base/common/Monitor.h"
#include "func.h"
#include "file_queue.h"

namespace neptune {
namespace dfs {

class FileQueueThread
{
  enum DESTROY_FLAG
  {
    DESTROY_FLAG_YES = 0x00,
    DESTROY_FLAG_NO
  };
  struct QueueThreadParam
  {
    int32_t thread_index_;
    FileQueueThread *queue_thread_;

  private:
    DISALLOW_COPY_AND_ASSIGN(QueueThreadParam);
  };

  typedef int (*deal_func)(const void * const data, const int64_t len, const int32_t thread_index, void* arg);

 public:
  FileQueueThread(FileQueue *queue, const void *arg);
  virtual ~FileQueueThread();
  int initialize(const int32_t thread_num, deal_func df);
  int write(const void* const data, const int32_t len);
  void wait();
  void destroy();
  void run(const int32_t thread_index);
  static void *thread_func(void *arg);

  inline void update_queue_information_header()
  {
    Monitor<Mutex>::Lock lock(monitor_);
    QueueInformationHeader head = *file_queue_->get_queue_information_header();
    //LOG(INFO, "Update QinfoHead(before): readSeqNo: %d, readOffset: %d, writeSeqNo: %d,"
    //  "writeFileSize: %d", head.read_seqno_, head.read_offset_, head.write_seqno_, head.write_filesize_);
    head.read_seqno_ = head.write_seqno_;
    head.read_offset_ = head.write_filesize_;
    file_queue_->update_queue_information_header(&head);
    //LOG(INFO, "Update QinfoHead(after): readSeqNo: %d, readOffset: %d, writeSeqNo: %d,"
    //  "writeFileSize: %d", head.read_seqno_, head.read_offset_, head.write_seqno_, head.write_filesize_);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN( FileQueueThread);

  void *args_;
  FileQueue *file_queue_;
  QueueThreadParam *queue_thread_param_;
  std::vector<pthread_t> pids_;
  deal_func deal_func_;
  DESTROY_FLAG destroy_;
  Monitor<Mutex> monitor_;
};

} //namespace dfs
} //namespace neptune

#endif
