#include "get_dataserver_information_message.h"

namespace neptune {
namespace dfs {

GetDataServerInformationMessage::GetDataServerInformationMessage():
  flag_(0)
{
  setPCode(GET_DATASERVER_INFORMATION_MESSAGE);
}
GetDataServerInformationMessage::~GetDataServerInformationMessage()
{

}
int GetDataServerInformationMessage::serialize(Stream& output) const 
{
  return output.set_int16(flag_);
}
int GetDataServerInformationMessage::deserialize(Stream& input)
{
  return input.get_int16(&flag_);
}
int64_t GetDataServerInformationMessage::length() const
{
  return INT16_SIZE;
}
GetDataServerInformationResponseMessage::GetDataServerInformationResponseMessage():
  bit_map_element_count_(0),
  data_length_(0),
  data_(NULL),
  flag_(0),
  alloc_(false)
{
  setPCode(GET_DATASERVER_INFORMATION_RESPONSE_MESSAGE);
}
GetDataServerInformationResponseMessage::~GetDataServerInformationResponseMessage()
{
  if (alloc_)
  {
    gDeleteA(data_);
  }
}
int GetDataServerInformationResponseMessage::serialize(Stream& output) const 
{
  int64_t pos = 0;
  int32_t iret = sblock_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    iret = info_.serialize(output.get_free(), output.get_free_length(), pos);
  }
  if (SUCCESS == iret)
  {
    output.pour(sblock_.length() + info_.length());
    iret = output.set_int16(flag_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(bit_map_element_count_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(data_length_); 
    if (SUCCESS == iret)
    {
      iret = output.set_bytes(data_, data_length_);
    }
  }
  return iret;
}

int GetDataServerInformationResponseMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = sblock_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    iret = info_.deserialize(input.get_data(), input.get_data_length(), pos);
  }
  if (SUCCESS == iret)
  {
    input.drain(sblock_.length() + info_.length());
    iret = input.get_int16(&flag_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&bit_map_element_count_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&data_length_);
    if (SUCCESS == iret)
    {
      if (data_length_ > 0)
      {
        data_ = input.get_data();
      }
    }
  }
  return iret;
}

int64_t GetDataServerInformationResponseMessage::length() const
{
  return sblock_.length() + info_.length() + INT16_SIZE + INT_SIZE * 2 + data_length_;
}

char* GetDataServerInformationResponseMessage::alloc_data(const int64_t length)
{
  char* data = NULL;
  if (length > 0)
  {
    gDeleteA(data_);
    data_length_ = length;
    alloc_  = true;
    data = data_ = new char[length];
  }
  return data;
}

} //namespace dfs
} //namespace neptune
