#include "compact_block_message.h"

namespace neptune {
namespace dfs {

CompactBlockMessage::CompactBlockMessage() :
  preserve_time_(0), block_id_(0)
{
  _packetHeader._pcode = COMPACT_BLOCK_MESSAGE;
}

CompactBlockMessage::~CompactBlockMessage()
{
}

int CompactBlockMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&preserve_time_);
  if (SUCCESS == iret)
  {
    iret = input.get_int32( reinterpret_cast<int32_t*> (&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&is_owner_);
  }
  if (SUCCESS == iret
    && input.get_data_length() > 0)
  {
    iret = input.get_int64(&seqno_);
  }
  return iret;
}


int64_t CompactBlockMessage::length() const
{
  return INT_SIZE * 3 + INT64_SIZE;
}

int CompactBlockMessage::serialize(Stream& output) const
{
  int32_t iret = output.set_int32(preserve_time_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(is_owner_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(seqno_);
  }
  return iret;
}

CompactBlockCompleteMessage::CompactBlockCompleteMessage() :
  block_id_(0), success_(PLAN_STATUS_END), server_id_(0), flag_(0)
{
  memset(&block_info_, 0, sizeof(block_info_));
  _packetHeader._pcode = BLOCK_COMPACT_COMPLETE_MESSAGE;
}

CompactBlockCompleteMessage::~CompactBlockCompleteMessage()
{
}

int CompactBlockCompleteMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&success_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int64(reinterpret_cast<int64_t*>(&server_id_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*>(&flag_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint64(ds_list_);
  }
  if (SUCCESS == iret)
  {
    int64_t pos = 0;
    iret = block_info_.deserialize(input.get_data(), input.get_data_length(), pos);
    if (SUCCESS == iret)
    {
      input.drain(block_info_.length());
    }
  }
  if (SUCCESS == iret
    && input.get_data_length() > 0)
  {
    iret = input.get_int64(&seqno_);
  }
  return iret;
}

int CompactBlockCompleteMessage::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &success_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&server_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&flag_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_vint64(data, data_len, pos, ds_list_);
  }
  if (SUCCESS == iret)
  {
    iret = block_info_.deserialize(data, data_len, pos);
  }
  if (SUCCESS == iret
      && pos + INT64_SIZE <= data_len)
  {
    iret = Serialization::get_int64(data, data_len, pos, &seqno_);
  }
  return iret;
}

int64_t CompactBlockCompleteMessage::length() const
{
  return  INT_SIZE * 3 + INT64_SIZE * 2 + block_info_.length() + Serialization::get_vint64_length(ds_list_);
}

int CompactBlockCompleteMessage::serialize(Stream& output) const
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(success_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(server_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(flag_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint64(ds_list_);
  }
  if (SUCCESS == iret)
  {
    int64_t pos = 0;
    iret = block_info_.serialize(output.get_free(), output.get_free_length(), pos);
    if (SUCCESS == iret)
    {
      output.pour(block_info_.length());
    }
  }
  if (SUCCESS == iret)
  {
    output.set_int64(seqno_);
  }
  return iret;
}
void CompactBlockCompleteMessage::dump(void) const
{
  std::string ipstr;
  VUINT64::const_iterator iter = ds_list_.begin();
  for (; iter != ds_list_.end(); ++iter)
  {
    ipstr += CNetUtil::addrToString((*iter));
    ipstr += "/";
  }
  //LOG(INFO, "compact block: %u, seqno: %"PRI64_PREFIX"d, success: %d, serverid: %"PRI64_PREFIX"u, flag: %d, block: %u, version: %u"
  //    "file_count: %u, size: %u, delfile_count: %u, del_size: %u, seqno: %u, ds_list: %zd, servers: %s",
  //    block_id_, seqno_, success_, server_id_, flag_, block_info_.block_id_, block_info_.version_, block_info_.file_count_, block_info_.size_,
  //    block_info_.del_file_count_, block_info_.del_size_, block_info_.seq_no_, ds_list_.size(), ipstr.c_str());
}

} //namespace dfs
} //namespace neptune
