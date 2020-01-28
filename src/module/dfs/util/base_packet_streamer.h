#ifndef N_DFS_UTIL_BASE_PACKET_STREAMER_H_
#define N_DFS_UTIL_BASE_PACKET_STREAMER_H_ 

#include "dfs/util/dfs.h"
#include "base/network/simple/net.h"

namespace neptune {
namespace dfs {

class BasePacketStreamer: public IPacketStreamer
{
 public:
  BasePacketStreamer();
  explicit BasePacketStreamer(IPacketFactory* factory);
  virtual ~BasePacketStreamer();
  void set_packet_factory(IPacketFactory* factory);

#ifdef NEP_GTEST
 public:
#else
 protected:
#endif

  virtual bool getPacketInfo(DataBuffer* input, PacketHeader* header, bool* broken);
  virtual Packet* decode(DataBuffer* input, PacketHeader* header);
  virtual bool encode(Packet* packet, DataBuffer* output);
};

} //namespace dfs
} //namespace neptune
#endif
