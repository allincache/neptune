#ifndef N_DFS_MESSAGE_COMPACTBLOCKMESSAGE_H
#define N_DFS_MESSAGE_COMPACTBLOCKMESSAGE_H

#include "base_task_message.h"
#include "base/common/Internal.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class CompactBlockMessage: public BaseTaskMessage
{
 public:
  CompactBlockMessage();
  virtual ~CompactBlockMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_preserve_time(const int32_t preserve_time)
  {
    preserve_time_ = preserve_time;
  }
  inline void set_block_id(const uint32_t block_id)
  {
    block_id_ = block_id;
  }
  inline void set_owner(const int32_t owner)
  {
    is_owner_ = owner;
  }
  inline int32_t get_preserve_time() const
  {
    return preserve_time_;
  }
  inline uint32_t get_block_id() const
  {
    return block_id_;
  }
  inline int32_t get_owner() const
  {
    return is_owner_;
  }
 protected:
  int32_t preserve_time_;
  uint32_t block_id_;
  int32_t is_owner_;
};

class CompactBlockCompleteMessage: public BaseTaskMessage
{
 public:
  CompactBlockCompleteMessage();
  virtual ~CompactBlockCompleteMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  int deserialize(const char* data, const int64_t data_len, int64_t& pos);
  virtual int64_t length() const;
  void dump(void) const;
  inline void set_block_id(const uint32_t block_id)
  {
    block_id_ = block_id;
  }
  inline uint32_t get_block_id() const
  {
    return block_id_;
  }
  inline void set_server_id(const uint64_t server_id)
  {
    server_id_ = server_id;
  }
  inline uint64_t get_server_id() const
  {
    return server_id_;
  }
  inline void set_block_info(const BlockInfo& block_info)
  {
    block_info_ = block_info;
  }
  inline BlockInfo& get_block_info()
  {
    return block_info_;
  }
  inline void set_success(const CompactStatus success)
  {
    success_ = success;
  }
  inline int32_t get_success() const
  {
    return success_;
  }
  inline void set_flag(const uint8_t flag)
  {
    flag_ = flag;
  }
  inline uint32_t get_flag() const
  {
    return flag_;
  }
  inline void set_ds_list(const VUINT64& ds_list)
  {
    ds_list_.assign(ds_list.begin(), ds_list.end());
  }
  inline const VUINT64& get_ds_list() const
  {
    return ds_list_;
  }
  
 protected:
  uint32_t block_id_;
  int32_t success_;
  uint64_t server_id_;
  BlockInfo block_info_;
  uint32_t flag_;
  VUINT64 ds_list_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_COMPACTBLOCKMESSAGE_H
