#ifndef N_DFS_MESSAGE_RELOADMESSAGE_H
#define N_DFS_MESSAGE_RELOADMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class ReloadConfigMessage: public BasePacket 
{
 public:
  ReloadConfigMessage();
  virtual ~ReloadConfigMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  void set_switch_cluster_flag(const int32_t flag);
  int32_t get_switch_cluster_flag() const;
 protected:
  int32_t flag_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_RELOADMESSAGE_H
