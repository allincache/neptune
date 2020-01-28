#include "read_data_message.h"

namespace neptune {
namespace dfs {

int ReadDataInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &length_);
  }
  return iret;
}
int ReadDataInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, length_);
  }
  return iret;
}
int64_t ReadDataInfo::length() const
{
  return INT_SIZE * 3 + INT64_SIZE;
}

ReadDataMessage::ReadDataMessage():
  flag_(READ_DATA_OPTION_FLAG_NORMAL)
{
  _packetHeader._pcode = READ_DATA_MESSAGE;
  memset(&read_data_info_, 0, sizeof(ReadDataInfo));
}

ReadDataMessage::~ReadDataMessage()
{
}

int ReadDataMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = read_data_info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(read_data_info_.length());
  }
  if (SUCCESS == iret)
  {
    input.get_int8(&flag_);
  }
  return iret;
}

int64_t ReadDataMessage::length() const
{
  return read_data_info_.length() + INT8_SIZE;
}

int ReadDataMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = read_data_info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(read_data_info_.length());
  }
  if (SUCCESS == iret)
  {
    output.set_int8(flag_);
  }
  return iret;
}

RespReadDataMessage::RespReadDataMessage() :
  data_(NULL), length_(-1), alloc_(false)
{
  _packetHeader._pcode = RESP_READ_DATA_MESSAGE;
}

RespReadDataMessage::~RespReadDataMessage()
{
  if ((NULL != data_ ) && (alloc_))
  {
    ::free(data_);
    data_ = NULL;
  }
}

char* RespReadDataMessage::alloc_data(const int32_t len)
{
  if (len < 0)
  {
    return NULL;
  }
  if (len == 0)
  {
    length_ = len;
    return NULL;
  }
  if (data_ != NULL)
  {
    ::free(data_);
    data_ = NULL;
  }
  length_ = len;
  data_ = (char*) malloc(len);
  alloc_ = true;
  return data_;
}

int RespReadDataMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&length_);
  if (SUCCESS == iret)
  {
    if (length_ > 0)
    {
      data_ = input.get_data();
      input.drain(length_);
    }
  }
  return iret;
}

int64_t RespReadDataMessage::length() const
{
  int64_t len = INT_SIZE;
  if (length_ > 0
    && NULL != data_)
  {
    len += length_;
  }
  return len;
}

int RespReadDataMessage::serialize(Stream& output) const
{
  int32_t iret = output.set_int32(length_);
  if (SUCCESS == iret)
  {
    if (length_ > 0
      && NULL != data_)
    {
      iret = output.set_bytes(data_, length_);
    }
  }
  return iret;
}

ReadDataMessageV2::ReadDataMessageV2():
  ReadDataMessage()
{
  _packetHeader._pcode = READ_DATA_MESSAGE_V2;
}

ReadDataMessageV2::~ReadDataMessageV2()
{
}

RespReadDataMessageV2::RespReadDataMessageV2():
  RespReadDataMessage()
{
  _packetHeader._pcode = RESP_READ_DATA_MESSAGE_V2;
  memset(&file_info_, 0, sizeof(file_info_));
}

RespReadDataMessageV2::~RespReadDataMessageV2()
{

}

int RespReadDataMessageV2::deserialize(Stream& input)
{
  int32_t iret = RespReadDataMessage::deserialize(input);
  if (SUCCESS == iret)
  {
    int32_t size = 0;
    iret = input.get_int32(&size);
    if (SUCCESS == iret
        && size > 0)
    {
      int64_t pos = 0;
      iret = file_info_.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        input.drain(file_info_.length());
      }
    }
  }
  return iret;
}

int64_t RespReadDataMessageV2::length() const
{
  int64_t len = RespReadDataMessage::length() + INT_SIZE;
  if (file_info_.id_ > 0)
  {
    len += file_info_.length();
  }
  return len;
}

int RespReadDataMessageV2::serialize(Stream& output) const
{
  int32_t iret = RespReadDataMessage::serialize(output);
  int32_t size = file_info_.id_ > 0 ? file_info_.length() : 0;
  if (SUCCESS == iret)
  {
    iret = output.set_int32(size);
  }
  if (SUCCESS == iret)
  {
    if (size > 0)
    {
      int64_t pos = 0;
      iret = file_info_.serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == iret)
      {
        output.pour(file_info_.length());
      }
    }
  }
  return iret;
}

ReadDataMessageV3::ReadDataMessageV3()
{
  _packetHeader._pcode = READ_DATA_MESSAGE_V3;
}

ReadDataMessageV3::~ReadDataMessageV3()
{

}

RespReadDataMessageV3::RespReadDataMessageV3()
{
  _packetHeader._pcode = RESP_READ_DATA_MESSAGE_V3;
}

RespReadDataMessageV3::~RespReadDataMessageV3()
{

}

ReadRawDataMessage::ReadRawDataMessage():
  ReadDataMessage()
{
  _packetHeader._pcode = READ_RAW_DATA_MESSAGE;
}

ReadRawDataMessage::~ReadRawDataMessage()
{

}
RespReadRawDataMessage::RespReadRawDataMessage() :
  RespReadDataMessage()
{
  _packetHeader._pcode = RESP_READ_RAW_DATA_MESSAGE;
}

RespReadRawDataMessage::~RespReadRawDataMessage()
{

}

int ReadScaleImageMessage::ZoomData::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &zoom_width_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &zoom_height_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &zoom_type_);
  }
  return iret;
}
int ReadScaleImageMessage::ZoomData::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, zoom_width_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, zoom_height_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, zoom_type_);
  }
  return iret;
}

int64_t ReadScaleImageMessage::ZoomData::length() const
{
  return INT_SIZE * 3;
}

ReadScaleImageMessage::ReadScaleImageMessage()
{
  _packetHeader._pcode = READ_SCALE_IMAGE_MESSAGE;
  memset(&zoom_, 0, sizeof(ZoomData));
}

ReadScaleImageMessage::~ReadScaleImageMessage()
{

}

int32_t ReadScaleImageMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = read_data_info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(read_data_info_.length());
    iret = zoom_.deserialize(input.get_data(), input.get_data_length(), pos);
    if (SUCCESS == iret)
    {
      input.drain(zoom_.length());
    }
  }
  return iret;
}

int32_t ReadScaleImageMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = read_data_info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(read_data_info_.length());
    iret = zoom_.serialize(output.get_free(), output.get_free_length(), pos);
    if (SUCCESS == iret)
    {
      output.pour(zoom_.length());
    }
  }
  return iret;
}

int64_t ReadScaleImageMessage::length() const
{
  return read_data_info_.length() + sizeof(ZoomData);
}

} //namespace dfs
} //namespace neptune
