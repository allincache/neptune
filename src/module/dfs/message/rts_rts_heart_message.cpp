#include "dfs/util/stream.h"
#include "base/common/Serialization.h"
#include "dfs/util/rts_define.h"
#include "rts_rts_heart_message.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

RtsRsHeartMessage::RtsRsHeartMessage():
  keepalive_type_(RTS_RS_KEEPALIVE_TYPE_LOGIN)
{
  _packetHeader._pcode = REQ_RT_RS_KEEPALIVE_MESSAGE;
}

RtsRsHeartMessage::~RtsRsHeartMessage()
{

}

int RtsRsHeartMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = server_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(server_.length());
    iret = input.get_int8(&keepalive_type_);
  }
  return iret;
}

int RtsRsHeartMessage::serialize(Stream& output) const 
{
  int64_t pos = 0;
  int32_t iret = server_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(server_.length());
    iret = output.set_int8(keepalive_type_);
  }
  return iret;
}

int64_t RtsRsHeartMessage::length() const
{
  return server_.length() + INT8_SIZE;
}

RtsRsHeartResponseMessage::RtsRsHeartResponseMessage():
  active_table_version_(INVALID_TABLE_VERSION),
  lease_expired_time_(RTS_RS_LEASE_EXPIRED_TIME_DEFAULT),
  ret_value_(ERROR),
  renew_lease_interval_time_(RTS_RS_RENEW_LEASE_INTERVAL_TIME_DEFAULT)
{
  _packetHeader._pcode = RSP_RT_RS_KEEPALIVE_MESSAGE;
}

RtsRsHeartResponseMessage::~RtsRsHeartResponseMessage()
{

}

int RtsRsHeartResponseMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int64(&active_table_version_);
  if (SUCCESS == iret)
  {
    iret = input.get_int64(&lease_expired_time_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&ret_value_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&renew_lease_interval_time_);
  }
  return iret;
}

int RtsRsHeartResponseMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int64(active_table_version_);
  if (SUCCESS == iret)
  {
    iret = output.set_int64(lease_expired_time_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(ret_value_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(renew_lease_interval_time_);
  }
  return iret;
}

int64_t RtsRsHeartResponseMessage::length() const
{
  return INT64_SIZE * 2 + INT_SIZE * 2;
}


} //namespace dfs
} //namespace neptune
