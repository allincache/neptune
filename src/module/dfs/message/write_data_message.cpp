#include "write_data_message.h"

namespace neptune {
namespace dfs {


WriteDataMessage::WriteDataMessage() :
  data_(NULL), version_(0), lease_id_(INVALID_LEASE_ID)
{
  _packetHeader._pcode = WRITE_DATA_MESSAGE;
  memset(&write_data_info_, 0, sizeof(write_data_info_));
  ds_.clear();
}

WriteDataMessage::~WriteDataMessage()
{

}

int WriteDataMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = write_data_info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain( write_data_info_.length());
    iret = input.get_vint64(ds_);
  }
  if (SUCCESS == iret
    && write_data_info_.length_ > 0)
  {
    data_ = input.get_data();
    input.drain(write_data_info_.length_);
  }
  if (SUCCESS == iret)
  {
    parse_special_ds(ds_, version_, lease_id_);
  }
  return iret;
}

int64_t WriteDataMessage::length() const
{
  int64_t len = write_data_info_.length() + Serialization::get_vint64_length(ds_);
  if (write_data_info_.length_ > 0)
  {
    len += write_data_info_.length_;
  }
  if (has_lease())
  {
    len += INT64_SIZE * 3;
  }
  return len;
}

int WriteDataMessage::serialize(Stream& output) const
{
  if (has_lease())
  {
    ds_.push_back(ULONG_LONG_MAX);
    ds_.push_back(static_cast<uint64_t>(version_));
    ds_.push_back(static_cast<uint64_t>(lease_id_));
  }
  int64_t pos = 0;
  int32_t iret = write_data_info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(write_data_info_.length());
    iret = output.set_vint64(ds_);
  }
  if (SUCCESS == iret
      && write_data_info_.length_ > 0)
  {
    iret = output.set_bytes(data_, write_data_info_.length_);
  }
  // reparse, avoid push verion&lease again when clone twice;
  if (SUCCESS == iret)
  {
    parse_special_ds(ds_, version_, lease_id_);
  }
  return iret;
}

#ifdef _DEL_001_
RespWriteDataMessage::RespWriteDataMessage():
  length_(0)
{
  _packetHeader._pcode = RESP_WRITE_DATA_MESSAGE;
}

RespWriteDataMessage::~RespWriteDataMessage()
{
}

int RespWriteDataMessage::deserialize(Stream& input)
{
  return input.get_int32(&length_);
}

int64_t RespWriteDataMessage::length() const
{
  return INT_SIZE;
}

int RespWriteDataMessage::serialize(Stream& output) const
{
  return output.set_int32(lenght_);
}

#endif

WriteRawDataMessage::WriteRawDataMessage() :
  data_(NULL), flag_(0)
{
  _packetHeader._pcode = WRITE_RAW_DATA_MESSAGE;
  memset(&write_data_info_, 0, sizeof(write_data_info_));
}

WriteRawDataMessage::~WriteRawDataMessage()
{

}

int WriteRawDataMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = write_data_info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(write_data_info_.length());
    if (write_data_info_.length_ > 0)
    {
      data_ = input.get_data();
      input.drain( write_data_info_.length_);
    }
  }

  if (SUCCESS == iret)
  {
    iret = input.get_int32(&flag_);
  }
  return iret;
}

int64_t WriteRawDataMessage::length() const
{
  int64_t len = write_data_info_.length() + INT_SIZE;
  if (write_data_info_.length_ > 0)
  {
    len += write_data_info_.length_;
  }
  return len;
}

int WriteRawDataMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = write_data_info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(write_data_info_.length());
    if (write_data_info_.length_ > 0)
    {
      iret = output.set_bytes(data_, write_data_info_.length_);
    }
  }

  if (SUCCESS == iret)
  {
    iret = output.set_int32(flag_);
  }
  return iret;
}

WriteInfoBatchMessage::WriteInfoBatchMessage() :
  cluster_(0)
{
  _packetHeader._pcode = WRITE_INFO_BATCH_MESSAGE;
  memset(&write_data_info_, 0, sizeof(write_data_info_));
  memset(&block_info_, 0, sizeof(block_info_));
  meta_list_.clear();
}

WriteInfoBatchMessage::~WriteInfoBatchMessage()
{

}

int WriteInfoBatchMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = write_data_info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(write_data_info_.length());
  }
  int32_t have_block = 0;
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&have_block);
  }

  if (SUCCESS == iret)
  {
    if (1 == have_block)
    {
      pos = 0;
      iret = block_info_.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        input.drain(block_info_.length());
      }
    }
  }

  int32_t size = 0;
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&size);
  }
  if (SUCCESS == iret)
  {
    for (int32_t i = 0; i < size; ++i)
    {
      pos = 0;
      RawMeta raw_meta;
      iret = raw_meta.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        meta_list_.push_back(raw_meta);
        input.drain(raw_meta.length());
      }
      else
      {
        break;
      }
    }
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&cluster_);
  }
  return iret;
}

int64_t WriteInfoBatchMessage::length() const
{
  int64_t len = write_data_info_.length() + INT_SIZE * 2;
  if (block_info_.block_id_ > 0)
  {
    len += block_info_.length();
  }
  len += INT_SIZE;
  RawMeta raw_data;
  len += meta_list_.size() * raw_data.length();
  return len;
}

int WriteInfoBatchMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t have_block = (block_info_.block_id_ > 0) ? 1 : 0;
  int32_t iret = write_data_info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(write_data_info_.length());
    iret = output.set_int32(have_block);
  }
  if (SUCCESS == iret)
  {
    if (1 == have_block)
    {
      pos = 0;
      iret = block_info_.serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == iret)
      {
        output.pour(block_info_.length());
      }
    }
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(meta_list_.size());
  }
  if (SUCCESS == iret)
  {
    RawMetaVec::const_iterator iter = meta_list_.begin();
    for (; iter != meta_list_.end(); ++iter)
    {
      pos = 0;
      iret = const_cast<RawMeta*>((&(*iter)))->serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == iret)
      {
        output.pour((*iter).length());
      }
      else
      {
        break;
      }
    }
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(cluster_);
  }
  return iret;
}


} //namespace dfs
} //namespace neptune
