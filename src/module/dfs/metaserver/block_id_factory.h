#ifndef N_DFS_MS_FSIMAGE_H
#define N_DFS_MS_FSIMAGE_H

#include "base/concurrent/Mutex.h"
#include "ns_define.h"

namespace neptune {
namespace dfs {
namespace metaserver {

class BlockIdFactory
{
 public:
  BlockIdFactory();
  virtual ~BlockIdFactory();
  int initialize(const std::string& path);
  int destroy();
  uint32_t generation(const uint32_t id = 0);
  uint32_t skip(const int32_t num = SKIP_BLOCK_NUMBER);
 
 private:
  int update(const uint32_t id) const;
  DISALLOW_COPY_AND_ASSIGN(BlockIdFactory);
  static BlockIdFactory instance_;
  Mutex mutex_;
  static const uint16_t BLOCK_START_NUMBER;
  static const uint16_t SKIP_BLOCK_NUMBER;
  uint32_t global_id_;
  int32_t  count_;
  int32_t  fd_;
};

}
}
}

#endif
