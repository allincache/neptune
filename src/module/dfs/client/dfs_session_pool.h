#ifndef N_DFS_CLIENT_DFSSESSION_POOL_H_
#define N_DFS_CLIENT_DFSSESSION_POOL_H_

#include <map>
#include "dfs/util/dfs.h"
#include "base/concurrent/Mutex.h"
#include "dfs_session.h"

namespace neptune {
namespace dfs {

#define SESSION_POOL DfsSessionPool::get_instance()
class DfsSessionPool
{
  typedef std::map<std::string, DfsSession*> SESSION_MAP;
 
 public:
  DfsSessionPool();
  virtual ~DfsSessionPool();

  inline static DfsSessionPool& get_instance()
  {
    return g_session_pool_;
  }

  DfsSession* get(const char* ns_addr, const int64_t cache_time = ClientConfig::cache_time_,
                  const int64_t cache_items = ClientConfig::cache_items_);
  void release(DfsSession* session);

 private:
  DISALLOW_COPY_AND_ASSIGN( DfsSessionPool);
  Mutex mutex_;
  SESSION_MAP pool_;
  static DfsSessionPool g_session_pool_;
};


}
}

#endif /* DFSSESSION_POOL_H_ */
