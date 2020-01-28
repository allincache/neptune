#ifndef N_DFS_MESSAGE_CHECKSERVERMESSAGE_H
#define N_DFS_MESSAGE_CHECKSERVERMESSAGE_H

#include <vector>
#include "dfs/util/base_packet.h"
#include "base/common/Internal.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class CheckBlockRequestMessage: public BasePacket
{
 public:
  CheckBlockRequestMessage(): check_flag_(0), block_id_(0),
        check_time_(0), last_check_time_(0)
  {
    _packetHeader._pcode = REQ_CHECK_BLOCK_MESSAGE;
  }
  virtual ~CheckBlockRequestMessage()
  {
  }
  virtual int serialize(Stream& output) const;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  void set_check_flag(const int32_t check_flag)
  {
    check_flag_ = check_flag;
  }

  int32_t get_check_flag()
  {
    return check_flag_;
  }

  void set_block_id(const uint32_t block_id)
  {
    block_id_ = block_id;
  }

  uint32_t get_block_id()
  {
    return block_id_;
  }

  void set_check_time(const uint32_t check_time)
  {
    check_time_ = check_time;
  }

  uint32_t get_check_time()
  {
    return check_time_;
  }

  void set_last_check_time(const uint32_t last_check_time)
  {
    last_check_time_ = last_check_time;
  }

  uint32_t get_last_check_time()
  {
    return last_check_time_;
  }

private:
  int32_t check_flag_;  // for scalability
  uint32_t block_id_;
  uint32_t check_time_;
  uint32_t last_check_time_;
};

class CheckBlockResponseMessage: public BasePacket
{
 public:
  CheckBlockResponseMessage()
  {
    _packetHeader._pcode = RSP_CHECK_BLOCK_MESSAGE;
  }
  virtual ~CheckBlockResponseMessage()
  {
  }
  virtual int serialize(Stream& output) const;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  CheckBlockInfoVec& get_result_ref()
  {
    return check_result_;
  }

 private:
  std::vector<CheckBlockInfo> check_result_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_CHECKSERVERMESSAGE_H
