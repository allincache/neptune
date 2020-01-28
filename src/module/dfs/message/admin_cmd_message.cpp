#include "admin_cmd_message.h"

namespace neptune {
namespace dfs {

int MonitorStatus::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_bytes(data, data_len, pos, index_, ADMIN_MAX_INDEX_LENGTH + 1);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &restarting_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &failure_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &pid_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &dead_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &start_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &dead_time_);
  }
  return iret;
}

int MonitorStatus::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_bytes(data, data_len, pos, index_, (ADMIN_MAX_INDEX_LENGTH + 1));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, restarting_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, failure_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, pid_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, dead_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, start_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, dead_time_);
  }
  return iret;
}

int64_t MonitorStatus::length() const
{
  return INT_SIZE * 6 + (ADMIN_MAX_INDEX_LENGTH + 1);
}

AdminCmdMessage::AdminCmdMessage() : type_(ADMIN_CMD_NONE)
{
  _packetHeader._pcode = ADMIN_CMD_MESSAGE;
}

AdminCmdMessage::AdminCmdMessage(int32_t cmd_type) : type_(cmd_type)
{
  _packetHeader._pcode = ADMIN_CMD_MESSAGE;
}

AdminCmdMessage::~AdminCmdMessage()
{
}

int AdminCmdMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&type_);
  if (SUCCESS == iret)
  {
    int32_t count = 0;
    iret = input.get_int32(&count);
    if (SUCCESS == iret)
    {
      if (ADMIN_CMD_RESP != type_)
      {
        std::string tmp;
        for (int32_t i = 0; i < count; i++)
        {
          iret = input.get_string(tmp);
          if (SUCCESS != iret)
            break;
          else
            index_.push_back(tmp);
        }
      }
      else
      {
        int64_t pos = 0;
        MonitorStatus status;
        for (int32_t i = 0; i < count; ++i)
        {
          pos = 0;
          iret = status.deserialize(input.get_data(), input.get_data_length(), pos);
          if (SUCCESS == iret)
          {
            input.drain(status.length());
            monitor_status_.push_back(status);
          }
          else
          {
            break;
          }
        }
      }
    }
  }
  return iret;
}

int AdminCmdMessage::serialize(Stream& output) const
{
  int32_t iret= output.set_int32(type_);
  if (SUCCESS == iret)
  {
    int32_t count = (ADMIN_CMD_RESP != type_) ? index_.size() : monitor_status_.size();
    iret = output.set_int32(count);
    if (SUCCESS == iret)
    {
      if (ADMIN_CMD_RESP != type_)
      {
        VSTRING::const_iterator iter = index_.begin();
        for (; iter != index_.end(); ++iter)
        {
          iret = output.set_string((*iter));
          if (SUCCESS != iret)
            break;
        }
      }
      else
      {
        std::vector<MonitorStatus>::const_iterator iter = monitor_status_.begin();
        for (; iter != monitor_status_.end(); ++iter)
        {
          int64_t pos = 0;
          iret = (*iter).serialize(output.get_free(), output.get_free_length(), pos);
          if (SUCCESS == iret)
            output.pour((*iter).length());
          else
            break;
        }
      }
    }
  }
  return iret;
}

int64_t AdminCmdMessage::length() const
{
  int64_t size = INT_SIZE * 2;
  if (ADMIN_CMD_RESP != type_)
  {
    VSTRING::const_iterator iter = index_.begin();
    for (; iter != index_.end(); ++iter)
    {
      size += Serialization::get_string_length((*iter));
    }
  }
  else
  {
    MonitorStatus st;
    size += st.length() * monitor_status_.size();
  }
  return size;
}

} //namespace dfs
} //namespace neptune
