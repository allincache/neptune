#ifndef N_DFS_MESSAGE_OPLOGSYNCMESSAGE_H
#define N_DFS_MESSAGE_OPLOGSYNCMESSAGE_H 

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class OpLogSyncMessage: public BasePacket 
{
 public:
  OpLogSyncMessage();
  virtual ~OpLogSyncMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  void set_data(const char* data, const int64_t length);
  inline int64_t get_length() const { return length_;}
  inline const char* get_data() const { return data_;}
 private:
  bool alloc_;
  int32_t length_;
  char* data_;
};

enum OpLogSyncMsgCompleteFlag
{
  OPLOG_SYNC_MSG_COMPLETE_YES = 0x00,
  OPLOG_SYNC_MSG_COMPLETE_NO
};

class OpLogSyncResponeMessage: public BasePacket 
{
 public:
  OpLogSyncResponeMessage();
  virtual ~OpLogSyncResponeMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline uint8_t get_complete_flag() const { return complete_flag_;}
  inline void set_complete_flag(uint8_t flag = OPLOG_SYNC_MSG_COMPLETE_YES) { complete_flag_ = flag;}
 private:
  uint8_t complete_flag_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_OPLOGSYNCMESSAGE_H
