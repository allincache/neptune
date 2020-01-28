#ifndef N_DFS_DS_DATAHANDLE_H_
#define N_DFS_DS_DATAHANDLE_H_

#include "physical_block.h"
#include "base/common/Internal.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class LogicBlock;
class DataHandle
{
 public:
  explicit DataHandle(LogicBlock* logic_blk) :
    logic_block_(logic_blk)
  {
  }

  ~DataHandle()
  {
  }

 public:
  int read_segment_info(FileInfo* inner_file_info, const int32_t offset);
  int write_segment_info(const FileInfo* inner_file_info, const int32_t offset);

  int write_segment_data(const char* buf, const int32_t nbytes, const int32_t offset);
  int read_segment_data(char* buf, const int32_t nbytes, const int32_t offset);

  int fadvise_readahead(const int64_t offset, const int64_t size);

 private:
  int choose_physic_block(PhysicalBlock** tmp_physical_block, const int32_t offset, int32_t& inner_offset,
      int32_t& inner_len);

  DataHandle();
  DISALLOW_COPY_AND_ASSIGN(DataHandle);

  LogicBlock* logic_block_; // associate logic block id
};

}
}
}

#endif //N_DFS_DS_DATAHANDLE_H_
