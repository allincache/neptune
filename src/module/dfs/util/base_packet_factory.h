#ifndef N_DFS_UTIL_BASE_PACKET_FACTORY_H_
#define N_DFS_UTIL_BASE_PACKET_FACTORY_H_

#include "dfs/util/dfs.h"
#include "base_packet.h"

namespace neptune {
namespace dfs {

class BasePacketFactory: public IPacketFactory
{
 public:
  BasePacketFactory(){}
  virtual ~BasePacketFactory(){}
  virtual Packet* createPacket(int pcode);
  virtual Packet* clone_packet(Packet* packet, const int32_t version, const bool deserialize); 
};

} //namespace dfs
} //namespace neptune
#endif
