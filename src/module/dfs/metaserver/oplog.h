#ifndef N_DFS_MS_OPLOG_H_
#define N_DFS_MS_OPLOG_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <ext/hash_map>
#include <errno.h>
#include <dirent.h>
#include "base/concurrent/Mutex.h"
#include "ns_define.h"
#include "dfs/util/file_queue.h"

namespace neptune {
namespace dfs {
namespace metaserver {

enum OpLogType
{
  OPLOG_TYPE_BLOCK_OP = 0x00,
  OPLOG_TYPE_REPLICATE_MSG,
  OPLOG_TYPE_COMPACT_MSG
};
#pragma pack(1)
struct OpLogHeader
{
  int serialize(char* data, const int64_t data_len, int64_t& pos) const;
  int deserialize(const char* data, const int64_t data_len, int64_t& pos);
  int64_t length() const;
  uint32_t seqno_;
  uint32_t time_;
  uint32_t crc_;
  uint16_t length_;
  int8_t  type_;
  int8_t  reserve_;
  char  data_[0];
};

struct OpLogRotateHeader
{
  int serialize(char* data, const int64_t data_len, int64_t& pos) const;
  int deserialize(const char* data, const int64_t data_len, int64_t& pos);
  int64_t length() const;
  uint32_t seqno_;
  int32_t rotate_seqno_;
  int32_t rotate_offset_;
};
#pragma pack()

struct BlockOpLog
{
  int serialize(char* data, const int64_t data_len, int64_t& pos) const;
  int deserialize(const char* data, const int64_t data_len, int64_t& pos);
  int64_t length() const;
  uint32_t seqno_;
  BlockInfo info_;
  VUINT32 blocks_;
  VUINT64 servers_;
  int8_t cmd_;
  void dump(const int32_t level) const;
};

class OpLog
{
 public:
  OpLog(const std::string& path, const int32_t max_log_slot_size = 0x400);
  virtual ~OpLog();
  int initialize();
  int update_oplog_rotate_header(const OpLogRotateHeader& head);
  int write(const uint8_t type, const char* const data, const int32_t length);
  inline void reset()
  {
    slots_offset_ = 0;
  }
  inline const char* const get_buffer() const
  {
    return buffer_;
  }
  inline int64_t get_slots_offset() const
  {
    return slots_offset_;
  }
  inline const OpLogRotateHeader* get_oplog_rotate_header() const
  {
    return &oplog_rotate_header_;
  }
 
 public:
  static int const MAX_LOG_SIZE = sizeof(OpLogHeader) + BLOCKINFO_SIZE + 1 + 64 * INT64_SIZE;
  const int MAX_LOG_SLOTS_SIZE;
  const int MAX_LOG_BUFFER_SIZE;
 
 private:
  OpLogRotateHeader oplog_rotate_header_;
  std::string path_;
  uint64_t seqno_;
  int64_t slots_offset_;
  int32_t fd_;
  char* buffer_;

 private:
  DISALLOW_COPY_AND_ASSIGN(OpLog);
};
  }//end namespace metaserver
}
}//end namespace neptune
#endif
