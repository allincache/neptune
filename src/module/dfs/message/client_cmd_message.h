#ifndef N_DFS_MESSAGE_CLIENTCMDMESSAGE_H
#define N_DFS_MESSAGE_CLIENTCMDMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

// Client Command
class ClientCmdMessage: public BasePacket 
{
 public:
  ClientCmdMessage();
  virtual ~ClientCmdMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_value1(const uint64_t value)
  {
    info_.value1_ = value;
  }
  inline uint64_t get_value1() const
  {
    return info_.value1_;
  }
  inline void set_value2(const uint64_t value)
  {
    info_.value2_ = value;
  }
  inline uint64_t get_value2() const
  {
    return info_.value2_;
  }
  inline void set_value3(const uint32_t value)
  {
    info_.value3_ = value;
  }
  inline uint32_t get_value3() const
  {
    return info_.value3_;
  }
  inline void set_value4(const uint32_t value)
  {
    info_.value4_ = value;
  }
  inline uint32_t get_value4() const
  {
    return info_.value4_;
  }
  inline void set_cmd(const int32_t cmd)
  {
    info_.cmd_ = cmd;
  }
  inline int32_t get_cmd() const
  {
    return info_.cmd_;
  }
  inline const ClientCmdInformation& get_cmd_info() const
  {
    return info_;
  }

 protected:
  ClientCmdInformation info_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_CLIENTCMDMESSAGE_H
