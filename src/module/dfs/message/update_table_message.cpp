#include "dfs/util/stream.h"
#include "base/common/Serialization.h"
#include "update_table_message.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

UpdateTableMessage::UpdateTableMessage():
  tables_(NULL),
  length_(-1),
  version_(INVALID_TABLE_VERSION),
  phase_(-1),
  alloc_(false)
{
  _packetHeader._pcode = REQ_RT_UPDATE_TABLE_MESSAGE;
}

UpdateTableMessage::~UpdateTableMessage()
{
  if (alloc_)
    gDeleteA(tables_);
}

char* UpdateTableMessage::alloc(const int64_t length)
{
  length_ = length < 0 ?  -1 : length == 0 ? 0 : length;
  gDeleteA(tables_);
  if (length > 0)
  {
    tables_ = new char[length_]; 
  }
  alloc_ = true;
  return tables_;
}

int UpdateTableMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int64(&version_);
  if (SUCCESS == iret)
  {
    iret = input.get_int8(&phase_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int64(&length_);
  }
  if (SUCCESS == iret)
  {
    if (length_ > 0
      && length_ <= MAX_BUCKET_ITEM_DEFAULT * INT64_SIZE)
    {
      char* data = alloc(length_);
      iret = input.get_bytes(data, length_);
    }
  }
  return iret;
}

int UpdateTableMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int64(version_);
  if (SUCCESS == iret)
  {
    iret = output.set_int8(phase_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(length_);
  }
  if (SUCCESS == iret)
  {
    if (length_ > 0
      && length_ <= MAX_BUCKET_ITEM_DEFAULT * INT64_SIZE
      && NULL != tables_)
    {
      iret = output.set_bytes(tables_, length_);
    }
  }
  return iret;
}

int64_t UpdateTableMessage::length() const
{
  return INT64_SIZE * 2 + INT8_SIZE + length_;
}

UpdateTableResponseMessage::UpdateTableResponseMessage():
  version_(INVALID_TABLE_VERSION),
  phase_(-1),
  status_(ERROR)
{
  _packetHeader._pcode = RSP_RT_UPDATE_TABLE_MESSAGE;
}

UpdateTableResponseMessage::~UpdateTableResponseMessage()
{

}

int UpdateTableResponseMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int64(&version_);
  if (SUCCESS == iret)
  {
    iret = input.get_int8(&phase_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int8(&status_);
  }
  return iret;
}

int UpdateTableResponseMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int64(version_);
  if (SUCCESS == iret)
  {
    iret = output.set_int8(phase_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int8(status_);
  }
  return iret;
}

int64_t UpdateTableResponseMessage::length() const
{
  return INT64_SIZE + INT8_SIZE * 2;
}


} //namespace dfs
} //namespace neptune
