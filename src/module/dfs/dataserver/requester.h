#ifndef N_DFS_DS_REQUESTER_H_
#define N_DFS_DS_REQUESTER_H_

#include "logic_block.h"
#include "data_management.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class Requester
{
 public:
  Requester() :
    dataserver_id_(0), ns_ip_port_(0), data_management_(NULL)
  {
  }

  ~Requester()
  {
  }

  int init(const uint64_t dataserver_id, const uint64_t ns_ip_port, DataManagement* data_management);
  int req_update_block_info(const uint32_t block_id, const UpdateBlockType repair = UPDATE_BLOCK_NORMAL);
  int req_block_write_complete(const uint32_t block_id,
      const int32_t lease_id, const int32_t success, const UnlinkFlag unlink_flag = UNLINK_FLAG_NO);

 private:
  DISALLOW_COPY_AND_ASSIGN(Requester);

  uint64_t dataserver_id_;
  uint64_t ns_ip_port_;
  DataManagement* data_management_;
};

}
}
}

#endif //N_DFS_DS_REQUESTER_H_
