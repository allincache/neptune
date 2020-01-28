#ifndef N_DFS_MESSAGE_REPLICATEBLOCKMESSAGE_H
#define N_DFS_MESSAGE_REPLICATEBLOCKMESSAGE_H

#include "base_task_message.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class ReplicateBlockMessage: public BaseTaskMessage
{
 public:
  ReplicateBlockMessage();
  virtual ~ReplicateBlockMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  int deserialize(const char* data, const int64_t data_len, int64_t& pos);
  virtual int64_t length() const;
  void dump(void) const;
  inline void set_command(const int32_t command)
  {
    command_ = command;
  }
  inline int32_t get_command() const
  {
    return command_;
  }
  inline ReplicateBlockMoveFlag get_move_flag() const
  {
    return static_cast<ReplicateBlockMoveFlag>(repl_block_.is_move_);
  }
  inline void set_expire(const int32_t expire)
  {
    expire_ = expire;
  }
  inline int32_t get_expire() const
  {
    return expire_;
  }
  inline void set_repl_block(const ReplBlock* repl_block)
  {
    if (NULL != repl_block)
    {
      repl_block_ = *repl_block;
    }
  }
  inline const ReplBlock* get_repl_block() const
  {
    return &repl_block_;
  }

 protected:
  int32_t command_;
  int32_t expire_;
  ReplBlock repl_block_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_REPLICATEBLOCKMESSAGE_H
