
#include "base/common/Memory.h"
#include "dfs/util/parameter.h"
#include "base/fs/DirectoryOp.h"
#include "base/fs/FsName.h"
#include "sync_base.h"
#include "dataservice.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::dfs;

SyncBase::SyncBase(DataService& service, const int32_t type, const int32_t index,
                    const char* src_addr, const char* dest_addr) :
  service_(service), backup_type_(type), src_addr_(src_addr), dest_addr_(dest_addr),
  is_master_(false), stop_(0), pause_(0), need_sync_(0), need_sleep_(0),
  file_queue_(NULL), backup_(NULL)
{
  if (src_addr != NULL &&
      strlen(src_addr) > 0 &&
      dest_addr != NULL &&
      strlen(dest_addr) > 0)
  {
    mirror_dir_ = dynamic_cast<DataService*>(DataService::instance())->get_real_work_dir() + "/mirror";
    uint64_t dest_ns_id = Func::get_host_ip(dest_addr);
    char queue_name[20];
    sprintf(queue_name, "queue_%" PRI64_PREFIX "u", dest_ns_id);
    // the 1st one is the master
    if (index == 0)
    {
      is_master_ = true;
    }
    file_queue_ = new FileQueue(mirror_dir_, queue_name);
    if (type == SYNC_TO_DFS_MIRROR)
    {
      backup_ = new DfsMirrorBackup(*this, src_addr, dest_addr);
    }
    //LOG(INFO, "backup type: %d, construct result: %d", type, backup_ ? 1 : 0);
  }
  else
  {}
    //LOG(ERROR, "sync mirror src addr or dest addr null!");
}

SyncBase::~SyncBase()
{
  gDelete(file_queue_);
  gDelete(backup_);
}

int SyncBase::init()
{
  int32_t ret = (!src_addr_.empty() && !dest_addr_.empty()) ? SUCCESS : ERROR;
  if (SUCCESS == ret)
  {
    ret = file_queue_->load_queue_head();
    if (SUCCESS == ret)
    {
      ret = file_queue_->initialize();
      if (SUCCESS == ret)
      {
        need_sync_ = backup_ ? backup_->init() : false;
        if (!need_sync_)
        {
          ret = ERROR;
        }
        else
        {
          // master need to recover second queue for compatibility
          if (is_master_)
          {
            ret = recover_second_queue();
          }
        }
      }
    }
  }
  return ret;
}

void SyncBase::stop()
{
  stop_ = 1;
  sync_mirror_monitor_.lock();
  sync_mirror_monitor_.notifyAll();
  sync_mirror_monitor_.unlock();
  retry_wait_monitor_.lock();
  retry_wait_monitor_.notifyAll();
  retry_wait_monitor_.unlock();

  // wait for thread join
  if (NULL != backup_)
    backup_->destroy();
}

int SyncBase::recover_second_queue()
{
  int ret = SUCCESS;
  // 1.check if secondqueue exist
  std::string queue_path = mirror_dir_ + "/secondqueue";
  if (!DirectoryOp::is_directory(queue_path.c_str()))
  {
    return SUCCESS;
  }

  // 2.init FileQueue
  FileQueue* second_file_queue = new FileQueue(mirror_dir_, "secondqueue");
  ret = second_file_queue->load_queue_head();
  if (SUCCESS == ret)
  {
    ret = second_file_queue->initialize();
    if (SUCCESS == ret)
    {
      // 3.move QueueItems from the 2nd queue to the 1st queue
      int32_t item_count = 0;
      while (!second_file_queue->empty())
      {
        QueueItem* item = NULL;
        item = second_file_queue->pop(0);
        if (NULL != item)
        {
          file_queue_->push(&(item->data_[0]), item->length_);
          free(item);
          item = NULL;
          item_count++;
        }
      }
      //LOG(DEBUG, "recover %d queue items from the second file queue success", item_count);

      // 4.delete FileQueue
      gDelete(second_file_queue);

      // 5.remove secondqueue directory
      ret = DirectoryOp::delete_directory_recursively(queue_path.c_str(), true) ? SUCCESS : ERROR;
    }
  }
  return ret;
}

int SyncBase::run_sync_mirror()
{
  int ret = SUCCESS;
  sync_mirror_monitor_.lock();
  while (0 == need_sync_ && 0 == stop_)
  {
    sync_mirror_monitor_.wait();
  }
  sync_mirror_monitor_.unlock();
  if (stop_)
  {
    return ret;
  }

  sync_mirror_monitor_.lock();
  need_sleep_ = 1;
  while (!stop_)
  {
    while (0 == stop_ && (pause_ || file_queue_->empty()))
    {
      sync_mirror_monitor_.wait();
    }
    if (stop_)
    {
      break;
    }
    QueueItem* item = file_queue_->pop(0);
    sync_mirror_monitor_.unlock();
    if (NULL != item)
    {
      if (SUCCESS == do_sync(&item->data_[0], item->length_))
      {
        file_queue_->finish(0);
      }
      free(item);
    }
    if (need_sleep_)
    {
      usleep(20000);
    }
    sync_mirror_monitor_.lock();
    if (need_sleep_ && file_queue_->empty())
    {
      need_sleep_ = 0;
    }
  }
  sync_mirror_monitor_.unlock();

  return SUCCESS;
}

int SyncBase::write_sync_log(const int32_t cmd, const uint32_t block_id, const uint64_t file_id, const uint64_t old_file_id)
{
  int32_t ret = block_id > 0 ? SUCCESS : ERROR;
  if (SUCCESS == ret)
  {
    if (stop_ || 0 == need_sync_)
    {
      //LOG(INFO, " no need write sync log, blockid: %u, fileid: %" PRI64_PREFIX "u, need sync: %d", block_id,
      //    file_id, need_sync_);
      return SUCCESS;
    }

    SyncData data;
    memset(&data, 0, sizeof(SyncData));
    data.cmd_ = cmd;
    data.block_id_ = block_id;
    data.file_id_ = file_id;
    data.old_file_id_ = old_file_id;
    data.retry_time_ = time(NULL);

    sync_mirror_monitor_.lock();
    ret = file_queue_->push(reinterpret_cast<void*>(&data), sizeof(SyncData));
    sync_mirror_monitor_.notify();
    sync_mirror_monitor_.unlock();

  }
  return ret;
}

int SyncBase::reset_log()
{
  if (stop_)
  {
    return SUCCESS;
  }
  sync_mirror_monitor_.lock();
  file_queue_->clear();
  need_sync_ = 1;
  sync_mirror_monitor_.notifyAll();
  sync_mirror_monitor_.unlock();
  //LOG(INFO, "sync log reset.");
  return SUCCESS;
}

int SyncBase::disable_log()
{
  if (stop_)
  {
    return SUCCESS;
  }
  sync_mirror_monitor_.lock();
  file_queue_->clear();
  need_sync_ = 0;
  sync_mirror_monitor_.notifyAll();
  sync_mirror_monitor_.unlock();
  //LOG(INFO, "sync log disable.");
  return SUCCESS;
}

void SyncBase::set_pause(const int32_t v)
{
  if (stop_)
  {
    return;
  }
  sync_mirror_monitor_.lock();
  if (pause_ != v && 0 == v)
  {
    need_sleep_ = 1;
  }
  pause_ = v;
  sync_mirror_monitor_.notifyAll();
  sync_mirror_monitor_.unlock();
  //LOG(INFO, "sync setpause: %d", pause_);
}

int SyncBase::do_sync(const char* data, const int32_t len)
{
  TIMER_START();
  if (NULL == data || len != sizeof(SyncData))
  {
    //LOG(WARN, "SYNC_ERROR: data null or len error, %d <> %zd", len, sizeof(SyncData));
    return ERROR;
  }
  SyncData* sf = reinterpret_cast<SyncData*>(const_cast<char*>(data));
  //LOG(DEBUG, "sync_begin. block_id: %u, file_id: %"PRI64_PREFIX"u,action: %d",
  //    sf->block_id_, sf->file_id_, sf->cmd_);
  int ret = backup_->do_sync(sf);
  TIMER_END();
  if (SUCCESS == ret)
  {
    //LOG(INFO, "sync_ok. block_id: %u, file_id: %"PRI64_PREFIX"u,action: %d, cost: %"PRI64_PREFIX"d",
    //    sf->block_id_, sf->file_id_, sf->cmd_, TIMER_DURATION());
  }
  else
  {
    // log to a file???
    FSName fsname(sf->block_id_, sf->file_id_);
    //LOG(ERROR, "sync_fail. block_id: %u, file_id: %"PRI64_PREFIX"u,"
    //    "action: %d, ret: %d, name:%s, cost: %"PRI64_PREFIX"d",
    //    sf->block_id_, sf->file_id_, sf->cmd_, ret, fsname.get_name(), TIMER_DURATION());
    }

  return SUCCESS;
}

}
}
}
