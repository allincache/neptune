#include "base/common/Memory.h"
#include "dfs_session_pool.h"

using namespace neptune::dfs;

DfsSessionPool DfsSessionPool::g_session_pool_;

DfsSessionPool::DfsSessionPool()
{
}

DfsSessionPool::~DfsSessionPool()
{
  Mutex::Lock lock(mutex_);
  SESSION_MAP::iterator it = pool_.begin();
  for (; it != pool_.end(); it++)
  {
    gDelete(it->second);
  }
  pool_.clear();
}

DfsSession* DfsSessionPool::get(const char* ns_addr, const int64_t cache_time, const int64_t cache_items)
{
  Mutex::Lock lock(mutex_);
  SESSION_MAP::iterator it = pool_.find(ns_addr);
  if (it != pool_.end())
  {
    return it->second;
  }
  DfsSession* session = new DfsSession(ns_addr, cache_time, cache_items);
  int32_t ret = session->initialize();
  if (ret != SUCCESS)
  {
    LOG(ERROR, "initialize dfs session failed, ret: %d", ret);
    gDelete(session);
    return NULL;
  }
  pool_[ns_addr] = session;
  return session;
}

void DfsSessionPool::release(DfsSession* session)
{
  Mutex::Lock lock(mutex_);
  if (session)
  {
    pool_.erase(session->get_ns_addr_str());
    gDelete(session);
  }
}
