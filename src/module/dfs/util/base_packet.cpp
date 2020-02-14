#include "dfs/util/Service.h"
#include "base_packet.h"
#include "base_service.h"
#include "status_message.h"

namespace neptune {
namespace dfs {

BasePacket::BasePacket():
  Packet(),
  connection_(NULL),
  id_(0),
  crc_(0),
  direction_(DIRECTION_SEND),
  version_(DFS_PACKET_VERSION_V2),
  //auto_free_(true),
  dump_flag_(false)
{

}

BasePacket::~BasePacket()
{

}

bool BasePacket::copy(BasePacket* src, const int32_t version, const bool deserialize)
{
  bool bret = NULL != src;
  if (bret)
  {
    bret = getPCode() == src->getPCode() && src != this;
    if (bret)
    {
      //base class
      _next = NULL;
      _channel = NULL;
      _expireTime = 0;
      memcpy(&_packetHeader, &src->_packetHeader, sizeof(PacketHeader));

      //self members
      id_ = src->get_id();
      crc_ = src->get_crc();
      version_ = version >= DFS_PACKET_VERSION_V0 ? version : src->get_version();
      //auto_free_ = src->get_auto_free();
      connection_ = src->get_connection();
      direction_  = src->get_direction();

      stream_.clear();
      stream_.expand(src->length());
      int32_t iret = src->serialize(stream_);
      bret = SUCCESS == iret;
      if (bret)
      {
        //recalculate crc
        if (version >= DFS_PACKET_VERSION_V1)
        {
          crc_ = Func::crc(DFS_PACKET_FLAG_V1, stream_.get_data(), stream_.get_data_length());
        }
        if (deserialize)
        {
          iret = this->deserialize(stream_);
          bret = SUCCESS == iret;
        }
      }
    }
  }
  return bret;
}

bool BasePacket::encode(DataBuffer* output)
{
  bool bret = NULL != output;
  if (bret)
  {
    if (stream_.get_data_length() > 0)
    {
      output->writeBytes(stream_.get_data(), stream_.get_data_length());
    }
  }
  return bret;
}

bool BasePacket::decode(DataBuffer* input, PacketHeader* header )
{
  bool bret = NULL != input && NULL != header;
  if (bret)
  {
    version_ = ((header->_pcode >> 16) & 0xFFFF);
    header->_pcode = (header->_pcode & 0xFFFF);
    int64_t length = header->_dataLen;
    bret = length > 0 && input->getDataLen() >= length;
    if (!bret)
    {
    //  LOG(ERROR, "invalid packet: %d, length: %"PRI64_PREFIX"d  input buffer length: %d",
      //   header->_pcode, length, input->getDataLen());
      input->clear();
    }
    else
    {
      //both v1 and v2 have id and crc
      if (version_ >= DFS_PACKET_VERSION_V1)
      {
        int64_t pos = 0;
        int32_t iret = Serialization::get_int64(input->getData(), input->getDataLen(), pos, reinterpret_cast<int64_t*>(&id_));
        if (SUCCESS != iret)
        {
          bret = false;
          input->drainData(length);
          // LOG(ERROR, "channel id deserialize error, iret: %d", iret);
        }
        else
        {
          iret = Serialization::get_int32(input->getData(), input->getDataLen(), pos, reinterpret_cast<int32_t*>(&crc_));
          if (SUCCESS != iret)
          {
            bret = false;
            input->drainData(length);
          //  LOG(ERROR, "crc deserialize error, iret: %d", iret);
          }
          else
          {
            length -= DFS_PACKET_HEADER_DIFF_SIZE;
            input->drainData(DFS_PACKET_HEADER_DIFF_SIZE);
            uint32_t crc = Func::crc(DFS_PACKET_FLAG_V1, input->getData(), length);
            bret = crc == crc_;
            if (!bret)
            {
              input->drainData(length);
            //  LOG(ERROR, "decode packet crc check error, header crc: %u, calc crc: %u",
            //      crc_, crc);
            }
          }
        }
      }
      if (bret)
      {
        stream_.clear();
        stream_.expand(length);
        stream_.set_bytes(input->getData(), length);
        //LOG(DEBUG, "decode packet pcode: %d", header->_pcode);
        //Func::hex_dump(input->getData(), length);
        input->drainData(length);
        int32_t iret = deserialize(stream_);
        bret = SUCCESS == iret;
        if (!bret)
        {
        //  LOG(ERROR, "packet: %d deserialize error, iret: %d", header->_pcode, iret);
        }
      }
    }
  }
  return bret;
}

int BasePacket::reply(BasePacket* packet)
{
  int32_t iret = NULL != packet ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    if (0 == getChannelId())
    {
    //  LOG(ERROR, "message : %d channel is null, reply message : %d", getPCode(), packet->getPCode());
      iret = EXIT_CHANNEL_ID_INVALID;
    }
    if (SUCCESS == iret)
    {
      if (((direction_ & DIRECTION_RECEIVE) && _expireTime > 0)
            && (CTimeUtil::getTime() > _expireTime))
      {
        // LOG(ERROR, "message : %d, timeout for response, reply message : %d", getPCode(), packet->getPCode());
        iret = EXIT_DATA_PACKET_TIMEOUT;
      }
    }

    if (SUCCESS == iret)
    {
      packet->setChannelId(getChannelId());
      packet->set_id(id_ + 1);
      packet->set_version(version_);

      packet->stream_.clear();
      packet->stream_.expand(packet->length());
      iret = packet->serialize(packet->stream_);
      //LOG(DEBUG, "reply, pcode: %d, %d", getPCode(), packet->getPCode());
      //Func::hex_dump(packet->stream_.get_data(), packet->stream_.get_data_length());
      if (SUCCESS == iret)
      {
        //recalculate crc
        if (version_ >= DFS_PACKET_VERSION_V1)
        {
          packet->crc_ = Func::crc(DFS_PACKET_FLAG_V1, packet->stream_.get_data(), packet->stream_.get_data_length());
        }
        if (is_enable_dump())
        {
          dump();
          packet->dump();
        }
        //post message
        bool bret= connection_->postPacket(packet);
        iret = bret ? SUCCESS : EXIT_SENDMSG_ERROR;
        if (SUCCESS != iret)
        {
          // LOG(ERROR, "post packet failure, server: %s, pcode:%d",
          //     CNetUtil::addrToString(connection_->getServerId()).c_str(), packet->getPCode());
        }
      }
      else
      {
        iret = EXIT_SERIALIZE_ERROR;
        //LOG(ERROR, "reply message failure, %d:%d, iret: %d", getPCode(), packet->getPCode(), iret);
      }
    }
    if (SUCCESS != iret)
    {
      packet->free();
    }
  }
  return iret;
}

int BasePacket::reply_error_packet(const int32_t level, const char* file, const int32_t line,
            const char* function, const int32_t error_code, const char* fmt, ...)
{
  char msgstr[MAX_ERROR_MSG_LENGTH + 1] = {'\0'};/** include '\0'*/
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(msgstr, MAX_ERROR_MSG_LENGTH, fmt, ap);
  va_end(ap);
  //LOGGER.logMessage(level, file, line, function, "%s, error code: %d", msgstr, error_code);

  BaseService* service = dynamic_cast<BaseService*>(BaseMain::instance());
  StatusMessage* packet = dynamic_cast<StatusMessage*>(service->get_packet_factory()->createPacket(STATUS_MESSAGE));
  msgstr[MAX_ERROR_MSG_LENGTH] = '\0';
  packet->set_message(error_code, msgstr);
  int32_t iret = reply(packet);
  if (SUCCESS != iret)
  {
    // LOG(ERROR, "reply message: %d failed, error code: %d", getPCode(), error_code);
  }
  return iret;
}

// parse for version & lease
// ds_: ds1 ds2 ds3 [flag=-1] [version] [leaseid]
bool BasePacket::parse_special_ds(std::vector<uint64_t>& value, int32_t& version, uint32_t& lease)
{
  bool bret = false;
  std::vector<uint64_t>::iterator iter =
          std::find(value.begin(), value.end(),static_cast<uint64_t> (ULONG_LONG_MAX));
  std::vector<uint64_t>::iterator start = iter;
  if ((iter != value.end())
    && ((iter + 2) < value.end()))
  {
    version = static_cast<int32_t>(*(iter + 1));
    lease   = static_cast<int32_t>(*(iter + 2));
    value.erase(start, value.end());
    bret = true;
  }
  return bret;
}

} //namespace dfs
} //namespace neptune
