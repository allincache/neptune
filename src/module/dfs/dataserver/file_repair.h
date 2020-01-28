#ifndef N_DFS_DS_FILEREPAIR_H_
#define N_DFS_DS_FILEREPAIR_H_

#include "dataserver_define.h"
#include "base/common/Internal.h"
#include "dfs/client/dfs_client_impl.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class FileRepair
{
 public:
  FileRepair();
  ~FileRepair();

  bool init(const uint64_t dataserver_id);
  int repair_file(const CrcCheckFile& crc_check_record, const char* tmp_file);
  int fetch_file(const CrcCheckFile& crc_check_record, char* tmp_file);

 private:
  static void get_tmp_file_name(char* buffer, const char* path, const char* name);
  int write_file(const int fd, const char* buffer, const int32_t length);

  bool init_status_;
  uint64_t dataserver_id_;
  char src_addr_[MAX_ADDRESS_LENGTH];
  DfsClientImpl* dfs_client_;
};

}
}
}

#endif //N_DFS_DS_FILEREPAIR_H_
