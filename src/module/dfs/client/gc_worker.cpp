#include <sys/file.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include "base/fs/DirectoryOp.h"
#include "dfs/util/func.h"
#include "gc_worker.h"
#include "base/fs/FsName.h"
#include "client_config.h"

using namespace neptune::base;
using namespace neptune::dfs;
using namespace std;

GcManager::GcManager()
   : destroy_(false),
     timer_(0),
     gc_worker_(0)
{
}

GcManager::~GcManager()
{
  gc_worker_ = 0;
}

int GcManager::initialize(TimerPtr timer, const int64_t schedule_interval_ms)
{
  int ret = ERROR;
  if (0 != timer)
  {
    timer_ = timer;
    gc_worker_ = new GcWorker();
    timer_->scheduleRepeated(gc_worker_, Time::milliSeconds(schedule_interval_ms));
    ret = SUCCESS;
  }
  return ret;
}

int GcManager::wait_for_shut_down()
{
  if (0 != timer_ && 0 != gc_worker_)
  {
    timer_->cancel(gc_worker_);
    timer_ = 0;
  }
  return SUCCESS;
}

int GcManager::destroy()
{
  if (0 != gc_worker_)
  {
    gc_worker_->destroy();
  }
  return SUCCESS;
}

int GcManager::reset_schedule_interval(const int64_t schedule_interval_ms)
{
  int ret = ERROR;
  if (0 != timer_ && 0 != gc_worker_)
  {
    timer_->cancel(gc_worker_);
    timer_->scheduleRepeated(gc_worker_, Time::milliSeconds(schedule_interval_ms));
    ret = SUCCESS;
  }
  return ret;
}

GcWorker::GcWorker()
{
}

GcWorker::~GcWorker()
{
}

void GcWorker::runTimerTask()
{
  //LOG(DEBUG, "gc start");
  int ret = SUCCESS;
  // gc expired local key and garbage gc file sequencially, maybe use thread

  // gc expired local key
  if ((ret = start_gc(GC_EXPIRED_LOCAL_KEY)) != SUCCESS)
  {
    //LOG(ERROR, "gc expired local key fail, ret: %d", ret);
  }
  else
  {
    //LOG(INFO, "gc expired local key success");
  }

  // gc file
  if ((ret = start_gc(GC_GARBAGE_FILE)) != SUCCESS)
  {
    //LOG(ERROR, "gc garbage file fail, ret: %d", ret);
  }
  else
  {
    //LOG(INFO, "gc garbage file success");
  }
  return;
}

int GcWorker::destroy()
{
  destroy_ = true;
  return SUCCESS;
}

int GcWorker::start_gc(const GcType gc_type)
{
  int ret = SUCCESS;
  if (!destroy_)
  {
    const char* gc_path = NULL;
    if (GC_EXPIRED_LOCAL_KEY == gc_type)
    {
      gc_path = "";//LOCAL_KEY_PATH;
    }
    else if (GC_GARBAGE_FILE == gc_type)
    {
      gc_path = "";//GC_FILE_PATH;
    }

    // private use, gc_type is valid
    if (!DirectoryOp::is_directory(gc_path))
    {
      //LOG(ERROR, "gc path doesn't exist: %s", gc_path);
      ret = ERROR;            // SUCCESS ?
    }
    else
    {
      // gc expired file
      if ((ret = get_expired_file(gc_path)) != SUCCESS)
      {
        //LOG(ERROR, "get expired file fail, ret: %d", ret);
      }
      else if (0 == file_.size())
      {
        //LOG(INFO, "no expired file, no gc");
      }
      else
      {
        if ((ret = do_gc(gc_type)) != SUCCESS)
        {
          //LOG(ERROR, "gc fail, ret: %d", ret);
        }
      }
    }
  }
  return ret;
}

int GcWorker::get_expired_file(const char* path)
{
  int ret = SUCCESS;
  DIR* dir = opendir(path);
  if (NULL == dir)
  {
    //LOG(ERROR, "can not open directory: %s, error: %s", path, strerror(errno));
    ret = ERROR;
  }
  else
  {
    file_.clear();
    struct dirent* dir_entry = readdir(dir);
    time_t now = time(NULL);
    while(dir_entry != NULL)
    {
      check_file(path, dir_entry->d_name, now);
      dir_entry = readdir(dir);
    }

    closedir(dir);
  }
  return ret;
}

int GcWorker::check_file(const char* path, const char* file, const time_t now)
{
  int ret = SUCCESS;
  char file_path[MAX_PATH_LENGTH];
  snprintf(file_path, MAX_PATH_LENGTH, "%s%s", path, file);

  struct stat64 file_info;
  if (stat64(file_path, &file_info) != 0)
  {
    //LOG(ERROR, "stat file fail: %s, error: %s", file_path, strerror(errno));
    ret = ERROR;
  }
  else if (S_ISREG(file_info.st_mode) && now - file_info.st_mtime > (ClientConfig::expired_time_ / 1000))
  {
    //LOG(INFO, "file need gc: %s, last modify time: %s",
    //          file_path, Func::time_to_str(file_info.st_mtime).c_str());
    file_.push_back(file_path);
  }
  else
  {
    //LOG(DEBUG, "file no need to gc: %s, last modify time: %s",
    //          file_path, Func::time_to_str(file_info.st_mtime).c_str());
  }
  return ret;
}

int GcWorker::do_gc(const GcType gc_type)
{
  int ret = SUCCESS;
  string::size_type id_pos = 0;

  //LOG(DEBUG, "gc file count: %zd", file_.size());
  for (size_t i = 0; i < file_.size(); i++)
  {
    string& file_name = file_[i];
    if (string::npos == (id_pos = file_name.rfind('!')))
    {
      //LOG(ERROR, "file name is not valid, no server id: %s", file_name.c_str());
    }
    else
    {
      int64_t file_size = 0;
      string addr = CNetUtil::addrToString(atoll(file_name.substr(id_pos + 1).c_str()));
      ////LOG(DEBUG, "id: %s, %"PRI64_PREFIX"d, server address %s",
      //          file_name.substr(id_pos).c_str(), atoll(file_name.substr(id_pos + 1)), addr.c_str());
      // expired local key
      if (GC_EXPIRED_LOCAL_KEY == gc_type)
      {
        if ((ret = do_gc_ex(local_key_, file_name.c_str(), addr.c_str(), file_size)) != SUCCESS)
        {
          //LOG(ERROR, "gc local key fail, file name: %s, ret: %d, file size: %"PRI64_PREFIX"d", file_name.c_str(), ret, file_size);
        }
      }
      // garbage file
      else if (GC_GARBAGE_FILE == gc_type)
      {
        if ((ret = do_gc_ex(gc_file_, file_name.c_str(), addr.c_str(), file_size)) != SUCCESS)
        {
          //LOG(ERROR, "gc garbage file fail, file name: %s, ret: %d, file size: %"PRI64_PREFIX"d", file_name.c_str(), ret, file_size);
        }
      }
    }
  }

  return ret;
}
