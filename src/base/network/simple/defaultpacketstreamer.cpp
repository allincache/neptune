#include "net.h"

namespace neptune {
namespace base {

int DefaultPacketStreamer::_nPacketFlag = NET_PACKET_FLAG;

DefaultPacketStreamer::DefaultPacketStreamer() {}

DefaultPacketStreamer::DefaultPacketStreamer(IPacketFactory *factory) : IPacketStreamer(factory) {}

DefaultPacketStreamer::~DefaultPacketStreamer() {}

void DefaultPacketStreamer::setPacketFactory(IPacketFactory *factory) {
  _factory = factory;
}

bool DefaultPacketStreamer::getPacketInfo(DataBuffer *input, PacketHeader *header, bool *broken) {
  if (_existPacketHeader) {
    if (input->getDataLen() < (int)(4 * sizeof(int)))
      return false;
    int flag = input->readInt32();
    header->_chid = input->readInt32();
    header->_pcode = input->readInt32();
    header->_dataLen = input->readInt32();
    if (flag != DefaultPacketStreamer::_nPacketFlag || header->_dataLen < 0 ||
      header->_dataLen > 0x4000000) { // 64M
      //LOG(ERROR, "stream error: %x<>%x, dataLen: %d", flag, DefaultPacketStreamer::_nPacketFlag, header->_dataLen);
      *broken = true;
    }
  } else if (input->getDataLen() == 0) {
    return false;
  }
  return true;
}

Packet *DefaultPacketStreamer::decode(DataBuffer *input, PacketHeader *header) {
  assert(_factory != NULL);
  Packet *packet = _factory->createPacket(header->_pcode);
  if (packet != NULL) {
    if (!packet->decode(input, header)) {
      packet->free();
      packet = NULL;
    }
  } else {
    input->drainData(header->_dataLen);
  }
  return packet;
}

bool DefaultPacketStreamer::encode(Packet *packet, DataBuffer *output) {
  PacketHeader *header = packet->getPacketHeader();

  int oldLen = output->getDataLen();
  int dataLenOffset = -1;
  int headerSize = 0;

  if (_existPacketHeader) {
    output->writeInt32(DefaultPacketStreamer::_nPacketFlag);
    output->writeInt32(header->_chid);
    output->writeInt32(header->_pcode);
    dataLenOffset = output->getDataLen();
    output->writeInt32(0);
    headerSize = 4 * sizeof(int);
  }
  if (packet->encode(output) == false) {
    //LOG(ERROR, "encode error");
    output->stripData(output->getDataLen() - oldLen);
    return false;
  }
  header->_dataLen = output->getDataLen() - oldLen - headerSize;
  if (dataLenOffset >= 0) {
    unsigned char *ptr = (unsigned char *)(output->getData() + dataLenOffset);
    output->fillInt32(ptr, header->_dataLen);
  }
  return true;
}

void DefaultPacketStreamer::setPacketFlag(int flag) {
  DefaultPacketStreamer::_nPacketFlag = flag;
}

} //namespace base
} //namespace neptune


