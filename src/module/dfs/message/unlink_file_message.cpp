#include "unlink_file_message.h"

namespace neptune {
namespace dfs {


int UnlinkFileInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &is_server_);
  }
  return iret;
}

int UnlinkFileInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_id_);
  }
  if (SUCCESS  == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, is_server_);
  }
  return iret;
}
int64_t UnlinkFileInfo::length() const
{
  return INT_SIZE * 2 + INT64_SIZE;
}

UnlinkFileMessage::UnlinkFileMessage() :
  option_flag_(0), version_(0), lease_id_(INVALID_LEASE_ID)
{
  _packetHeader._pcode = UNLINK_FILE_MESSAGE;
  memset(&unlink_file_info_, 0, sizeof(UnlinkFileInfo));
}

UnlinkFileMessage::~UnlinkFileMessage()
{
}

int UnlinkFileMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = unlink_file_info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(unlink_file_info_.length());
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint64(dataservers_);
  }
  if (SUCCESS == iret)
  {
    input.get_int32(&option_flag_);
  }
  if (SUCCESS == iret)
  {
    BasePacket::parse_special_ds(dataservers_, version_, lease_id_);
  }
  return iret;
}

int64_t UnlinkFileMessage::length() const
{
  int64_t len = unlink_file_info_.length() + Serialization::get_vint64_length(dataservers_) + INT_SIZE;
  if (has_lease())
  {
    len += INT64_SIZE * 3;
  }
  return len;
}

int UnlinkFileMessage::serialize(Stream& output) const
{
  if (has_lease())
  {
    dataservers_.push_back(ULONG_LONG_MAX);
    dataservers_.push_back(static_cast<uint64_t> (version_));
    dataservers_.push_back(static_cast<uint64_t> (lease_id_));
  }

  int64_t pos = 0;
  int32_t iret = unlink_file_info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(unlink_file_info_.length());
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint64(dataservers_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(option_flag_);
  }
  if (SUCCESS == iret)
  {
    BasePacket::parse_special_ds(dataservers_, version_, lease_id_);
  }
  return iret;
}


} //namespace dfs
} //namespace neptune
