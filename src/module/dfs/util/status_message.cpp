#include "status_message.h"

namespace neptune {
namespace dfs {

StatusMessage::StatusMessage() :
  length_(0),
  status_(STATUS_MESSAGE_ERROR)
{
  msg_[0] = '\0';
  _packetHeader._pcode = STATUS_MESSAGE;
}

StatusMessage::StatusMessage(const int32_t status, const char* const str):
  length_(0),
  status_(status)
{
  msg_[0] = '\0';
  _packetHeader._pcode = STATUS_MESSAGE;
  set_message(status, str);
}

void StatusMessage::set_message(const int32_t status, const char* const str)
{
  status_ = status;
  if (NULL != str)
  {
    int64_t length = strlen(str);
    if (length > MAX_ERROR_MSG_LENGTH)
    {
      length = MAX_ERROR_MSG_LENGTH;
    }
    memcpy(msg_, str, length);
    msg_[length] = '\0';
    length_ = length;
  }
  //LOG(DEBUG, "status msg : status: %d, length: %ld, %p", status_, length_, str);
}

StatusMessage::~StatusMessage()
{

}

int StatusMessage::serialize(Stream& output) const
{
  int32_t iret = output.set_int32(status_);
  if (SUCCESS == iret)
  {
    iret = output.set_string(msg_);
  }
  return iret;
}

int StatusMessage::deserialize(Stream& input)
{
  //Func::hex_dump(input.get_data(), input.get_data_length());
  int32_t iret = input.get_int32(&status_);
  if (SUCCESS == iret)
  {
    iret = input.get_string(MAX_ERROR_MSG_LENGTH, msg_, length_);
    if (SUCCESS == iret)
    {
      msg_[length_] = '\0';
    }
  }
  return iret;
}

int64_t StatusMessage::length() const
{
  return INT_SIZE + Serialization::get_string_length(msg_);
}

} //namespace dfs
} //namespace neptune
