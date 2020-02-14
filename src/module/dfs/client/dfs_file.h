#ifndef N_DFS_CLIENT_DfsFile_H_
#define N_DFS_CLIENT_DfsFile_H_

#include <sys/types.h>
#include <fcntl.h>

#include "base/common/ErrorMsg.h"
#include "dfs/util/func.h"
#include "base/concurrent/Lock.h"
#include "base/common/Internal.h"
#include "dfs/util/base_packet.h"
#include "dfs_session.h"
#include "base/fs/FsName.h"
#include "local_key.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class NewClient;
enum InnerFilePhase
{
  FILE_PHASE_OPEN_FILE = 0,
  FILE_PHASE_CREATE_FILE,
  FILE_PHASE_WRITE_DATA,
  FILE_PHASE_CLOSE_FILE,
  FILE_PHASE_READ_FILE,
  FILE_PHASE_READ_FILE_V2,
  FILE_PHASE_STAT_FILE,
  FILE_PHASE_UNLINK_FILE
};

struct PhaseStatus
{
  SegmentStatus pre_status_;
  SegmentStatus status_;
};

// CAUTION: depend on InnerFilePhase member sequence, maybe change to map
const static PhaseStatus phase_status[] = {
  {SEG_STATUS_NOT_INIT, SEG_STATUS_OPEN_OVER},            // dummy open.
  {SEG_STATUS_OPEN_OVER, SEG_STATUS_CREATE_OVER},         // create.
  {SEG_STATUS_CREATE_OVER, SEG_STATUS_BEFORE_CLOSE_OVER}, // write
  {SEG_STATUS_BEFORE_CLOSE_OVER, SEG_STATUS_ALL_OVER}, // close. just read write stat unlink is same previous phase
  {SEG_STATUS_OPEN_OVER, SEG_STATUS_ALL_OVER},         // read.
  {SEG_STATUS_OPEN_OVER, SEG_STATUS_ALL_OVER},         // readV2.
  {SEG_STATUS_OPEN_OVER, SEG_STATUS_ALL_OVER},         // stat.
  {SEG_STATUS_OPEN_OVER, SEG_STATUS_ALL_OVER}          // unlink.
};

enum
{
  DFS_FILE_OPEN_YES = 0,
  DFS_FILE_OPEN_NO,
  DFS_FILE_WRITE_ERROR
};

class DfsFile
{
public:
  DfsFile();
  virtual ~DfsFile();

  // virtual interface
  virtual int open(const char* file_name, const char* suffix, const int flags, ... ) = 0;
  virtual int64_t read(void* buf, const int64_t count) = 0;
  virtual int64_t readv2(void* buf, const int64_t count, DfsFileStat* file_info) = 0;
  virtual int64_t write(const void* buf, const int64_t count) = 0;
  virtual int64_t lseek(const int64_t offset, const int whence) = 0;
  virtual int64_t pread(void* buf, const int64_t count, const int64_t offset) = 0;
  virtual int64_t pwrite(const void* buf, const int64_t count, const int64_t offset) = 0;
  virtual int fstat(DfsFileStat* file_info, const DfsStatType mode = NORMAL_STAT) = 0;
  virtual int close() = 0;
  virtual int64_t get_file_length() = 0;
  virtual int unlink(const char* file_name, const char* suffix, int64_t& file_size, const DfsUnlinkType action) = 0;

  const char* get_file_name(const bool simple = false);
  void set_session(DfsSession* dfs_session);
  void set_option_flag(OptionFlag option_flag);

protected:
  // virtual level operation
  virtual int64_t get_segment_for_read(const int64_t offset, char* buf, const int64_t count) = 0;
  virtual int64_t get_segment_for_write(const int64_t offset, const char* buf, const int64_t count) = 0;
  virtual int read_process(int64_t& read_size, const InnerFilePhase file_phase) = 0;
  virtual int write_process() = 0;
  virtual int finish_write_process(const int status) = 0;
  virtual int close_process() = 0;
  virtual int unlink_process() = 0;
  virtual int wrap_file_info(DfsFileStat* file_stat, FileInfo* file_info) = 0;

protected:
  // common operation
  void destroy_seg();
  int64_t get_meta_segment(const int64_t offset, const char* buf, const int64_t count, const bool force_check = true);
  int process(const InnerFilePhase file_phase);
  int read_process_ex(int64_t& read_size, const InnerFilePhase read_file_phase);
  int stat_process();
  int32_t finish_read_process(const int status, int64_t& read_size);

  int get_block_info(SegmentData& seg_data, const int32_t flags);
  int get_block_info(SEG_DATA_LIST& seg_data_list, const int32_t flags);

  int open_ex(const char* file_name, const char *suffix, const int32_t flags);
  int64_t read_ex(void* buf, const int64_t count, const int64_t offset,
                  const bool modify = true, const InnerFilePhase read_file_phase = FILE_PHASE_READ_FILE);
  int64_t write_ex(const void* buf, const int64_t count, const int64_t offset, const bool modify = true);
  int64_t lseek_ex(const int64_t offset, const int whence);
  int64_t pread_ex(void* buf, const int64_t count, const int64_t offset);
  int64_t pwrite_ex(const void* buf, const int64_t count, const int64_t offset);
  int fstat_ex(FileInfo* file_info, const DfsStatType mode);
  int close_ex();

private:
  int process_fail_response(NewClient* client);
  int process_success_response(const InnerFilePhase file_phase, NewClient* client);

  int do_async_request(const InnerFilePhase file_phase, NewClient* client, const uint16_t index);
  int do_async_response(const InnerFilePhase file_phase, BasePacket* packet, const uint16_t index);

  int async_req_create_file(NewClient* client, const uint16_t index);
  int async_rsp_create_file(BasePacket* packet, const uint16_t index);

  int async_req_read_file(NewClient* client, const uint16_t index,
                          SegmentData& seg_data, BasePacket& rd_message);
  int async_req_read_file(NewClient* client, const uint16_t index);
  int async_rsp_read_file(BasePacket* packet, const uint16_t index);

  int async_req_read_file_v2(NewClient* client, const uint16_t index);
  int async_rsp_read_file_v2(BasePacket* packet, const uint16_t index);

  int async_req_write_data(NewClient* client, const uint16_t index);
  int async_rsp_write_data(BasePacket* packet, const uint16_t index);

  int async_req_close_file(NewClient* client, const uint16_t index);
  int async_rsp_close_file(BasePacket* packet, const uint16_t index);

  int async_req_stat_file(NewClient* client, const uint16_t index);
  int async_rsp_stat_file(BasePacket* packet, const uint16_t index);

  int async_req_unlink_file(NewClient* client, const uint16_t index);
  int async_rsp_unlink_file(BasePacket* packet, const uint16_t index);
#ifdef DFS_TEST
  bool get_process_result(uint32_t block_id, VUINT64& ds);
#endif

public:
  RWLock rw_lock_;

protected:
  FSName fsname_;
  int32_t flags_;
  int32_t file_status_;
  int32_t eof_;
  int64_t offset_;
  SegmentData* meta_seg_;
  int32_t option_flag_;
  DfsSession* dfs_session_;
  SEG_DATA_LIST processing_seg_list_;
  std::map<uint8_t, uint16_t> send_id_index_map_;
};

}
}

#endif  // N_DFS_CLIENT_DfsFile_H_
