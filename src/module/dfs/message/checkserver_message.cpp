#include "checkserver_message.h"

namespace neptune {
namespace dfs {

int CheckBlockRequestMessage::serialize(Stream& output) const
{
  int ret = SUCCESS;
  if (SUCCESS == ret)
  {
    ret = output.set_int32(check_flag_);
  }

  if (SUCCESS == ret)
  {
    ret = output.set_int32(block_id_);
  }

  if (SUCCESS == ret)
  {
    ret = output.set_int32(check_time_);
  }

  if (SUCCESS == ret)
  {
    ret = output.set_int32(last_check_time_);
  }

  return ret;
}

int CheckBlockRequestMessage::deserialize(Stream& input)
{
  int ret = SUCCESS;
  if (SUCCESS == ret)
  {
    ret = input.get_int32((int32_t*)&check_flag_);
  }

  if (SUCCESS == ret)
  {
    ret = input.get_int32((int32_t*)&block_id_);
  }

  if (SUCCESS == ret)
  {
    ret = input.get_int32((int32_t*)&check_time_);
  }

  if (SUCCESS == ret)
  {
    ret = input.get_int32((int32_t*)&last_check_time_);
  }

  return ret;
}

int64_t CheckBlockRequestMessage::length() const
{
  return 3 * INT_SIZE;
}

int CheckBlockResponseMessage::serialize(Stream& output) const
{
  int ret = output.set_int32(check_result_.size());
  if (SUCCESS == ret)
  {
    for (uint32_t i = 0; i < check_result_.size(); i++)
    {
      int64_t pos = 0;
      ret = check_result_[i].serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == ret)
      {
        output.pour(check_result_[i].length());
      }
      else
      {
        break;
      }
    }
  }
  return ret;
}

int CheckBlockResponseMessage::deserialize(Stream& input)
{
  int32_t size = 0;
  int ret = input.get_int32(&size);
  if (SUCCESS == ret)
  {
    CheckBlockInfo cbi;
    for (int i = 0; i < size; i++)
    {
      int64_t pos = 0;
      ret = cbi.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == ret)
      {
        check_result_.push_back(cbi);
        input.drain(cbi.length());
      }
      else
      {
        break;
      }
    }
  }
  return ret;
}

int64_t CheckBlockResponseMessage::length() const
{
  CheckBlockInfo cbi;
  return INT_SIZE + check_result_.size() * cbi.length();
}
  
} //namespace dfs
} //namespace neptune