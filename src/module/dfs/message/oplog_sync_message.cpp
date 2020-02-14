#include "base/common/Memory.h"
#include "oplog_sync_message.h"

namespace neptune {
namespace dfs {

OpLogSyncMessage::OpLogSyncMessage() :
  alloc_(false), length_(0), data_(NULL)
{
  _packetHeader._pcode = OPLOG_SYNC_MESSAGE;
}

OpLogSyncMessage::~OpLogSyncMessage()
{
  if ((data_ != NULL) && alloc_)
  {
    gDeleteA(data_);
  }
}

void OpLogSyncMessage::set_data(const char* data, const int64_t length)
{
  assert(length > 0);
  assert(data != NULL);
  gDeleteA(data_);
  length_ = length;
  data_ = new char[length];
  memcpy(data_, data, length);
  alloc_ = true;
}

int OpLogSyncMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&length_);
  if (SUCCESS == iret)
  {
    iret = length_ > 0 && length_ <= input.get_data_length() ? SUCCESS : ERROR;
    if (SUCCESS == iret)
    {
      set_data(input.get_data(), length_);
      input.drain(length_);
    }
  }
  return iret;
}

int64_t OpLogSyncMessage::length() const
{
  return (length_ > 0 && NULL != data_ ) ? INT_SIZE + length_ : INT_SIZE;
}

int OpLogSyncMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int32(length_);
  if (SUCCESS == iret)
  {
    if (length_ > 0 && NULL != data_)
    {
      iret = output.set_bytes(data_, length_);
    }
  }
  return iret;
}

OpLogSyncResponeMessage::OpLogSyncResponeMessage() :
  complete_flag_(OPLOG_SYNC_MSG_COMPLETE_YES)
{
  _packetHeader._pcode = OPLOG_SYNC_RESPONSE_MESSAGE;
}

OpLogSyncResponeMessage::~OpLogSyncResponeMessage()
{

}

int OpLogSyncResponeMessage::deserialize(Stream& input)
{
  return input.get_int8(reinterpret_cast<int8_t*>(&complete_flag_));
}

int64_t OpLogSyncResponeMessage::length() const
{
  return INT8_SIZE;
}

int OpLogSyncResponeMessage::serialize(Stream& output) const 
{
  return output.set_int8(complete_flag_);
}

} //namespace base
} //namespace neptune
