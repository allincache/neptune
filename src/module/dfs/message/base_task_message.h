#ifndef N_DFS_MESSAGE_BASE_TASK_MESSAGE_H
#define N_DFS_MESSAGE_BASE_TASK_MESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class BaseTaskMessage: public BasePacket
{
 public:
  BaseTaskMessage(){}
  virtual ~BaseTaskMessage(){}
  virtual int serialize(Stream& output) const = 0;
  virtual int deserialize(Stream& input) = 0;
  virtual int64_t length() const = 0;
  virtual void dump(void) const {};
  int64_t get_seqno() const { return seqno_;}
  void set_seqno(const int64_t seqno) { seqno_ = seqno;}
 protected:
  int64_t seqno_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_BASE_TASK_MESSAGE_H
