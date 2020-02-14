#include "rollback_message.h"

namespace neptune {
namespace dfs {


RollbackMessage::RollbackMessage() :
  act_type_(0)
{
  memset(&block_info_, 0, block_info_.length());
  memset(&file_info_, 0, file_info_.length());
  _packetHeader._pcode = ROLLBACK_MESSAGE;
}

RollbackMessage::~RollbackMessage()
{
}

int RollbackMessage::deserialize(Stream& input)
{
  int32_t block_len = 0;
  int32_t file_info_len = 0;
  int32_t iret = input.get_int32(&act_type_);
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&block_len);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&file_info_len);
  }
  int64_t pos = 0;
  if (SUCCESS == iret)
  {
    if (block_len > 0)
    {
      iret = block_info_.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        input.drain(block_info_.length());
      }
    }
  }
  
  if (SUCCESS == iret)
  {
    if (file_info_len > 0)
    {
      pos = 0;
      iret = file_info_.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        input.drain(file_info_.length());
      }
    }
  }
  return iret;
}

int64_t RollbackMessage::length() const
{
  int64_t len = INT_SIZE * 3;
  if (block_info_.block_id_ > 0)
    len += block_info_.length();
  if (file_info_.id_ > 0)
    len += file_info_.length();
  return len;
}

int RollbackMessage::serialize(Stream& output) const 
{
  int32_t block_len = block_info_.block_id_ > 0 ? block_info_.length() : 0;
  int32_t file_info_len = file_info_.id_ > 0 ? file_info_.length() : 0;
  int32_t iret = output.set_int32(act_type_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(block_len);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(file_info_len);
  }
  int64_t pos = 0;
  if (SUCCESS == iret)
  {
    if (block_len > 0)
    {
      iret = block_info_.serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == iret)
      {
        output.pour(block_info_.length());
      }
    }
  }
  if (SUCCESS == iret)
  {
    if (file_info_len > 0)
    {
      pos = 0;
      iret = file_info_.serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == iret)
      {
        output.pour(file_info_.length());
      }
    }
  }
  return iret;
}


} //namespace dfs
} //namespace neptune
