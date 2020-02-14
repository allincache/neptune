#ifndef N_DFS_CLIENT_GCFILE_H_
#define N_DFS_CLIENT_GCFILE_H_

#include "base/common/Internal.h"
#include "base/fs/FileOp.h"
#include "segment_container.h"

namespace neptune {
namespace dfs {

const int32_t GC_BATCH_WIRTE_COUNT = 10;
//extern const char* GC_FILE_PATH;
const mode_t GC_FILE_PATH_MODE = 0777;

class GcFile : public SegmentContainer< std::vector<SegmentInfo> >
{
 public:
  explicit GcFile(const bool need_save_seg_infos = true);
  virtual ~GcFile();

  virtual int load();
  virtual int add_segment(const SegmentInfo& seg_info);
  virtual int validate(const int64_t total_size = 0);
  virtual int save();

  int initialize(const char* name);

 private:
  DISALLOW_COPY_AND_ASSIGN(GcFile);
  int save_gc();
  int load_head();
  bool need_save_seg_infos_;
  int file_pos_;
};

}
}

#endif
