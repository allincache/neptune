#ifndef N_DFS_MESSAGE_DUMP_PLAN_MESSAGE_H
#define N_DFS_MESSAGE_DUMP_PLAN_MESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class DumpPlanMessage: public BasePacket 
{
 public:
  DumpPlanMessage();
  virtual ~DumpPlanMessage();
  virtual int serialize(Stream& output) const;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
 private:
  int8_t reserve_;
};

class DumpPlanResponseMessage: public BasePacket 
{
 public:
  DumpPlanResponseMessage();
  virtual ~DumpPlanResponseMessage();
  virtual int serialize(Stream& output) const;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline DataBuffer& get_data()
  {
    return data_;
  }
 private:
  mutable DataBuffer data_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_DUMP_PLAN_MESSAGE_H
