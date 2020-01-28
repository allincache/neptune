#include "dfs/util/stream.h"
#include "base/common/Serialization.h"

#include "get_tables_from_rts_message.h"

namespace neptune {
namespace dfs {

GetTableFromRtsResponseMessage::GetTableFromRtsResponseMessage():
  tables_(new char[MAX_BUCKET_DATA_LENGTH]),
  length_(MAX_BUCKET_DATA_LENGTH),
  version_(INVALID_TABLE_VERSION)
{
  _packetHeader._pcode = RSP_RT_GET_TABLE_MESSAGE;
}

GetTableFromRtsResponseMessage::~GetTableFromRtsResponseMessage()
{
  gDeleteA(tables_);
}

int GetTableFromRtsResponseMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int64(&version_);
  if (SUCCESS == iret)
  {
    iret = input.get_int64(&length_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_bytes(tables_, length_);
  }
  return iret;
}

int GetTableFromRtsResponseMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int64(version_);
  if (SUCCESS == iret)
  {
    iret = output.set_int64(length_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_bytes(tables_, length_);
  }
  return iret;
}

int64_t GetTableFromRtsResponseMessage::length() const
{
  return INT64_SIZE * 2 + length_;
}

GetTableFromRtsMessage::GetTableFromRtsMessage():
  reserve_(0)
{
  _packetHeader._pcode = REQ_RT_GET_TABLE_MESSAGE;
}

GetTableFromRtsMessage::~GetTableFromRtsMessage()
{

}

int GetTableFromRtsMessage::deserialize(Stream& input)
{
  return input.get_int8(&reserve_);
}

int GetTableFromRtsMessage::serialize(Stream& output) const 
{
  return output.set_int8(reserve_);
}

int64_t GetTableFromRtsMessage::length() const
{
  return INT8_SIZE;
}

} //namespace dfs
} //name
