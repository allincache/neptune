#include "dfs/util/dfs.h"
#include "gc.h"
#include "base/common/ErrorMsg.h"
#include "dfs/util/config_item.h"

namespace neptune {
namespace dfs {
namespace dataserver {

GCObjectManager GCObjectManager::instance_;

GCObjectManager::GCObjectManager():
  is_init_(false), destroy_(false)
{

}

GCObjectManager::~GCObjectManager()
{

}

int GCObjectManager::add(GCObject* object)
{
  bool bret = object == NULL ? false : destroy_ ? false : true;
  if (bret)
  {
    Mutex::Lock lock(mutex_);
    std::list<GCObject*>::const_iterator iter = std::find(object_list_.begin(), object_list_.end(), object);
    if (iter == object_list_.end())
    {
      object_list_.push_back(object);
    }
  }
  return bret ? SUCCESS : ERROR;
}

void GCObjectManager::run()
{
  int64_t now = time(NULL);
  GCObject* obj = NULL;
  Mutex::Lock lock(mutex_);
  std::list<GCObject*>::iterator iter = object_list_.begin();
  while (iter != object_list_.end() && !destroy_)
  {
    if ((*iter)->is_dead(now))
    {
      obj = (*iter);
      obj->callback();
      obj->free();
      object_list_.erase(iter++);
    }
    else
    {
      ++iter;
    }
  }
}

int GCObjectManager::initialize(TimerPtr timer)
{
  ExpireTimerTaskPtr task = new ExpireTimerTask(*this);
  int iret = timer->scheduleRepeated(task, GC_WORKER_INTERVAL);
  is_init_ = true;
  return iret < 0 ? ERROR : SUCCESS;
}

int GCObjectManager::wait_for_shut_down()
{
  Mutex::Lock lock(mutex_);
  std::list<GCObject*>::iterator iter = object_list_.begin();
  for (; iter != object_list_.end(); ++iter)
  {
    (*iter)->callback();
    (*iter)->free();
  }
  object_list_.clear();
  return SUCCESS;
}

void GCObjectManager::destroy()
{
  destroy_ = true;
}

void GCObjectManager::ExpireTimerTask::runTimerTask()
{
  manager_.run();
}

}
}
}
