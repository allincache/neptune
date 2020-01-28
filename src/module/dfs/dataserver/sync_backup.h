#ifndef N_DFS_DS_SYNCBACKUP_H_
#define N_DFS_DS_SYNCBACKUP_H_

#include "base/common/Internal.h"
#include "dfs/client/dfs_client_impl.h"
#include "base/common/Memory.h"
#include "base/thread/ThreadDetail.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;

enum SyncType
{
  SYNC_TO_DFS_MIRROR = 1,
};

struct SyncData
{
  int32_t cmd_;
  uint32_t block_id_;
  uint64_t file_id_;
  uint64_t old_file_id_;
  int32_t retry_count_;
  int32_t retry_time_;
};

class SyncBase;
class SyncBackup
{
public:
  SyncBackup();
  virtual ~SyncBackup();

  virtual bool init() = 0;
  virtual void destroy() = 0;
  virtual int do_sync(const SyncData* sf);
  virtual int copy_file(const uint32_t block_id, const uint64_t file_id);
  virtual int remove_file(const uint32_t block_id, const uint64_t file_id, const int32_t undel);
  virtual int rename_file(const uint32_t block_id, const uint64_t file_id, const uint64_t old_file_id);
  virtual int remote_copy_file(const uint32_t block_id, const uint64_t file_id);

protected:
  DISALLOW_COPY_AND_ASSIGN(SyncBackup);
  DfsClientImpl* dfs_client_;

  char src_addr_[MAX_ADDRESS_LENGTH];
  char dest_addr_[MAX_ADDRESS_LENGTH];
};

class DfsMirrorBackup : public SyncBackup
{
  public:
    DfsMirrorBackup(SyncBase& sync_base, const char* src_addr, const char* dest_addr);
    virtual ~DfsMirrorBackup();

    bool init();
    void destroy();
    int do_sync(const SyncData* sf);

  private:
    DISALLOW_COPY_AND_ASSIGN(DfsMirrorBackup);

  private:
    int copy_file(const uint32_t block_id, const uint64_t file_id);
    int remove_file(const uint32_t block_id, const uint64_t file_id, const DfsUnlinkType action);
    int rename_file(const uint32_t block_id, const uint64_t file_id, const uint64_t old_file_id);
    int remote_copy_file(const uint32_t block_id, const uint64_t file_id);
    int get_file_info(const char* nsip, const char* file_name, DfsFileStat& buf);

    /**
    * @brief sync file by stat
    *
    * sync file to same stat in both clusters
    *
    * @param block_id: block id
    * @param file_id:  file id
    *
    * @return
    */
    int sync_stat(const uint32_t block_id, const uint64_t file_id);


    /**
     * @brief if file not exist
     *
     * @param ret: access return value
     *
     * @return
     */
    bool file_not_exist(int ret);

  class DoSyncMirrorThreadHelper: public Thread
  {
    public:
      explicit DoSyncMirrorThreadHelper(SyncBase& sync_base):
          sync_base_(sync_base)
      {
        start();
      }
      virtual ~DoSyncMirrorThreadHelper(){}
      void run();
    private:
      DISALLOW_COPY_AND_ASSIGN(DoSyncMirrorThreadHelper);
      SyncBase& sync_base_;
  };
  typedef Handle<DoSyncMirrorThreadHelper> DoSyncMirrorThreadHelperPtr;

 private:
  SyncBase& sync_base_;
  DoSyncMirrorThreadHelperPtr  do_sync_mirror_thread_;

};

}
}
}

#endif //N_DFS_DS_SYNCBACKUP_H_
