#include "base_packet_factory.h"
#include "local_packet.h"
#include "status_message.h"

namespace neptune {
namespace dfs {

Packet* BasePacketFactory::createPacket(int pcode)
{
  Packet* packet = NULL;
  int real_pcode = (pcode & 0xFFFF);
  switch (real_pcode)
  {
  case LOCAL_PACKET:
    packet = new LocalPacket();
    break;
  case STATUS_MESSAGE:
    packet = new StatusMessage();
    break;
  }
  return packet;
}

Packet* BasePacketFactory::clone_packet(Packet* packet, const int32_t version, const bool deserialize) 
{
  Packet* clone_packet = NULL;
  if (NULL != packet)
  {
    clone_packet = createPacket(packet->getPCode());
    if (NULL != clone_packet)
    {
      BasePacket* bpacket = dynamic_cast<BasePacket*>(clone_packet);
      LOG(DEBUG, "pcode: %d, length: %" PRI64_PREFIX "d", bpacket->getPCode(), bpacket->length());
      bool bret = bpacket->copy(dynamic_cast<BasePacket*>(packet), version, deserialize);
      if (!bret)
      {
        LOG(ERROR, "clone packet error, pcode: %d", packet->getPCode());
        //bpacket->set_auto_free();
        bpacket->free();
        clone_packet = NULL;
      }
    }
  }
  return clone_packet;
}

} //namespace dfs
} //namespace neptune
