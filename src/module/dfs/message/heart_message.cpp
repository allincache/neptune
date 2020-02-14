#include "base/common/Serialization.h"
#include "dfs/util/stream.h"
#include "dfs/util/parameter.h"
#include "heart_message.h"

namespace neptune {
namespace dfs {

RespHeartMessage::RespHeartMessage() :
  status_(0),
  heart_interval_(DEFAULT_HEART_INTERVAL)
{
  _packetHeader._pcode = RESP_HEART_MESSAGE;
  expire_blocks_.clear();
}

RespHeartMessage::~RespHeartMessage()
{

}

int RespHeartMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&status_);
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&heart_interval_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint32(expire_blocks_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint32(new_blocks_);
  }
  return iret;
}

int64_t RespHeartMessage::length() const
{
  return INT_SIZE * 2 +
            Serialization::get_vint32_length(expire_blocks_) +
              Serialization::get_vint32_length(new_blocks_);
}

int RespHeartMessage::serialize(Stream& output) const
{
  int32_t iret = output.set_int32(status_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(heart_interval_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint32(expire_blocks_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint32(new_blocks_);
  }
  return iret;
}

int NSIdentityNetPacket::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, ip_port_);
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int8(data, data_len, pos, role_);
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int8(data, data_len, pos, status_);
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int8(data, data_len, pos, flags_);
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int8(data, data_len, pos, force_);
  }
  return iret;
}

int NSIdentityNetPacket::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&ip_port_));
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int8(data, data_len, pos, reinterpret_cast<int8_t*>(&role_));
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int8(data, data_len, pos, reinterpret_cast<int8_t*>(&status_));
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int8(data, data_len, pos, reinterpret_cast<int8_t*>(&flags_));
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int8(data, data_len, pos, reinterpret_cast<int8_t*>(&force_));
  }
  return iret;
}

int64_t NSIdentityNetPacket::length() const
{
  return INT64_SIZE + INT8_SIZE * 4;
}

MasterAndSlaveHeartMessage::MasterAndSlaveHeartMessage():
  lease_id_(INVALID_LEASE_ID),
  keepalive_type_(0)
{
  _packetHeader._pcode = MASTER_AND_SLAVE_HEART_MESSAGE;
  memset(&ns_identity_, 0, sizeof(ns_identity_));
}

MasterAndSlaveHeartMessage::~MasterAndSlaveHeartMessage()
{

}

int MasterAndSlaveHeartMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = ns_identity_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(ns_identity_.length());
  }
  if (SUCCESS == iret)
  {
    if (input.get_data_length() > 0)
      iret = input.get_int64(&lease_id_);
  }
  if (SUCCESS == iret)
  {
    if (input.get_data_length() > 0)
      iret = input.get_int8(&keepalive_type_);
  }
  return iret;
}

int MasterAndSlaveHeartMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = ns_identity_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(ns_identity_.length());
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(lease_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int8(keepalive_type_);
  }
  return iret;
}

int64_t MasterAndSlaveHeartMessage::length() const
{
  return ns_identity_.length() + INT64_SIZE + INT8_SIZE;
}

MasterAndSlaveHeartResponseMessage::MasterAndSlaveHeartResponseMessage():
  lease_id_(INVALID_LEASE_ID),
  lease_expired_time_(SYSPARAM_NAMESERVER.heart_interval_),
  renew_lease_interval_time_(SYSPARAM_NAMESERVER.heart_interval_)
{
  _packetHeader._pcode = MASTER_AND_SLAVE_HEART_RESPONSE_MESSAGE;
  ::memset(&ns_identity_, 0, sizeof(ns_identity_));
}

MasterAndSlaveHeartResponseMessage::~MasterAndSlaveHeartResponseMessage()
{

}

int MasterAndSlaveHeartResponseMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = ns_identity_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(ns_identity_.length());
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int64(&lease_id_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&lease_expired_time_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&renew_lease_interval_time_);
  }
  return iret;
}

int MasterAndSlaveHeartResponseMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = ns_identity_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(ns_identity_.length());
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(lease_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(lease_expired_time_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(renew_lease_interval_time_);
  }
  return iret;
}

int64_t MasterAndSlaveHeartResponseMessage::length() const
{
  return ns_identity_.length() + INT64_SIZE + INT_SIZE * 2;
}

HeartBeatAndNSHeartMessage::HeartBeatAndNSHeartMessage() :
  flags_(0)
{
  _packetHeader._pcode = HEARTBEAT_AND_NS_HEART_MESSAGE;
}

HeartBeatAndNSHeartMessage::~HeartBeatAndNSHeartMessage()
{

}

int HeartBeatAndNSHeartMessage::deserialize(Stream& input)
{
  return input.get_int32(&flags_);
}

int HeartBeatAndNSHeartMessage::serialize(Stream& output) const
{
  return output.set_int32(flags_);
}

int64_t HeartBeatAndNSHeartMessage::length() const
{
  return INT_SIZE;
}

/*OwnerCheckMessage::OwnerCheckMessage() :
  start_time_(0)
{
  _packetHeader._pcode = OWNER_CHECK_MESSAGE;
}

OwnerCheckMessage::~OwnerCheckMessage()
{

}*/

} //namespace dfs
} //namespace neptune
