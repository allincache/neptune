#ifndef N_DFS_DS_SYNCBASE_H_
#define N_DFS_DS_SYNCBASE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <errno.h>
#include "base/common/Monitor.h"
#include "base/concurrent/Mutex.h"
#include "sync_backup.h"
#include "dfs/util/file_queue.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class DataService;
class SyncBase
{
 public:
  SyncBase(DataService& service, const int32_t type, const int32_t index, const char* src_addr, const char* dest_addr);
  ~SyncBase();
  int init();
  void stop();

  int write_sync_log(const int32_t cmd, const uint32_t block_id, const uint64_t file_id, const uint64_t old_file_id = 0);
  int reset_log();
  int disable_log();
  void set_pause(const int32_t v);
  int run_sync_mirror();

 private:
  SyncBase();
  DISALLOW_COPY_AND_ASSIGN(SyncBase);

  DataService& service_;
  int32_t backup_type_;
  std::string mirror_dir_;
  std::string src_addr_;
  std::string dest_addr_;
  bool is_master_;      // master is responsible to sync to the other backup clusters
  bool stop_;
  int32_t pause_;
  int32_t need_sync_;
  int32_t need_sleep_;
  Monitor<Mutex> sync_mirror_monitor_;
  Monitor<Mutex> retry_wait_monitor_;
#if defined(NEP_GTEST)
 public:
#else
#endif
  FileQueue* file_queue_;
  SyncBackup* backup_;

 private:
  int do_sync(const char* data, const int32_t len);
  int recover_second_queue();
};

  }
}
}

#endif //N_DFS_DS_SYNCBASE_H_
