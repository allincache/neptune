#include "block_info_message.h"

namespace neptune {
namespace dfs {

int SdbmStat::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &startup_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &fetch_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &miss_fetch_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &store_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &delete_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &overflow_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &page_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &item_count_);
  }
  return iret;
}

int SdbmStat::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, startup_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, fetch_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, miss_fetch_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, store_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, delete_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, overflow_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, page_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, item_count_);
  }
  return iret;
}

int64_t SdbmStat::length() const
{
  return INT_SIZE * 8;
}

GetBlockInfoMessage::GetBlockInfoMessage(const int32_t mode) :
  block_id_(0), mode_(mode)
{
  _packetHeader._pcode = GET_BLOCK_INFO_MESSAGE;
}

GetBlockInfoMessage::~GetBlockInfoMessage()
{
}

int GetBlockInfoMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&mode_);
  if (SUCCESS == iret)
  {
    iret = input.get_int32( reinterpret_cast<int32_t*> (&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint64(fail_server_);
  }
  return iret;
}

int64_t GetBlockInfoMessage::length() const
{
  return INT_SIZE * 2 + Serialization::get_vint64_length(fail_server_);
}

int GetBlockInfoMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(mode_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint64(fail_server_);
  }
  return iret;
}

SetBlockInfoMessage::SetBlockInfoMessage() :
  block_id_(0), version_(0), lease_id_(INVALID_LEASE_ID)
{
  _packetHeader._pcode = SET_BLOCK_INFO_MESSAGE;
  ds_.clear();
}

SetBlockInfoMessage::~SetBlockInfoMessage()
{
}

//block_id, server_count, server_id1, server_id2, ..., filename_len, filename
int SetBlockInfoMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_vint64(ds_);
  }
  if (SUCCESS == iret)
  {
    BasePacket::parse_special_ds(ds_, version_, lease_id_);
  }
  return iret;
}

int64_t SetBlockInfoMessage::length() const
{
  int64_t len = INT_SIZE + Serialization::get_vint64_length(ds_);
  if (has_lease()
    && !ds_.empty())
  {
    len += INT64_SIZE * 3;
  }
  return len;
}

int SetBlockInfoMessage::serialize(Stream& output)  const
{
  if (has_lease()
    && !ds_.empty())
  {
    ds_.push_back(ULONG_LONG_MAX);
    ds_.push_back(static_cast<uint64_t> (version_));
    ds_.push_back(static_cast<uint64_t> (lease_id_));
  }
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_vint64(ds_);
  }
  if (SUCCESS == iret)
  {
    // reparse, avoid push verion&lease again when clone twice;
    BasePacket::parse_special_ds(ds_, version_, lease_id_);
  }
  return iret;
}

void SetBlockInfoMessage::set(const uint32_t block_id, const int32_t version, const uint32_t lease_id)
{
  block_id_ = block_id;
  if (lease_id != INVALID_LEASE_ID)
  {
    version_ = version;
    lease_id_ = lease_id;
  }
}

BatchGetBlockInfoMessage::BatchGetBlockInfoMessage(int32_t mode) :
  mode_(mode), block_count_(0)
{
  _packetHeader._pcode = BATCH_GET_BLOCK_INFO_MESSAGE;
  block_ids_.clear();
}

BatchGetBlockInfoMessage::~BatchGetBlockInfoMessage()
{
}

int BatchGetBlockInfoMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&mode_);
  if (SUCCESS == iret)
  {
    if (mode_ & T_READ)
    {
      iret = input.get_vint32(block_ids_);
    }
    else
    {
      iret = input.get_int32(&block_count_);
    }
  }
  return iret;
}

int64_t BatchGetBlockInfoMessage::length() const
{
  int64_t len = INT_SIZE;
  return (mode_ & T_READ) ? len + Serialization::get_vint32_length(block_ids_) : len + INT_SIZE;
}

int BatchGetBlockInfoMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(mode_);
  if (SUCCESS == iret)
  {
    if (mode_ & T_READ)
    {
      iret = output.set_vint32(block_ids_);
    }
    else
    {
      iret = output.set_int32(block_count_);
    }
  }
  return iret;
}

BatchSetBlockInfoMessage::BatchSetBlockInfoMessage()
{
  _packetHeader._pcode = BATCH_SET_BLOCK_INFO_MESSAGE;
}

BatchSetBlockInfoMessage::~BatchSetBlockInfoMessage()
{
}

// count, blockid, server_count, server_id1, server_id2, ..., blockid, server_count, server_id1 ...
int BatchSetBlockInfoMessage::deserialize(Stream& input)
{
  int32_t count = 0;
  int32_t iret = input.get_int32(&count);
  if (SUCCESS == iret)
  {
    uint32_t block_id = 0;
    for (int32_t i = 0; i < count; ++i)
    {
      BlockInfoSeg block_info;
      iret = input.get_int32(reinterpret_cast<int32_t*>(&block_id));
      if (SUCCESS != iret)
          break;
      iret = input.get_vint64(block_info.ds_);
      if (SUCCESS != iret)
          break;
      BasePacket::parse_special_ds(block_info.ds_, block_info.version_, block_info.lease_id_);
      block_infos_[block_id] = block_info;
    }
  }
  return iret;
}

int64_t BatchSetBlockInfoMessage::length() const
{
  int64_t len = INT_SIZE * block_infos_.size();
  // just test first has lease, then all has lease, maybe add mode test
  if (!block_infos_.empty())
  {
    std::map<uint32_t, BlockInfoSeg>::const_iterator it = block_infos_.begin();
    for (; it != block_infos_.end(); it++)
    {
      len += Serialization::get_vint64_length(it->second.ds_);
    }
    if (block_infos_.begin()->second.has_lease())
    {
      // has_lease + lease + version
      len += INT64_SIZE * 3 * block_infos_.size();
    }
  }
  return len;
}

int BatchSetBlockInfoMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(block_infos_.size());
  if (SUCCESS == iret)
  {
    std::map<uint32_t, BlockInfoSeg>::const_iterator it = block_infos_.begin();
    BlockInfoSeg* block_info = NULL;
    for (; it != block_infos_.end(); it++)
    {
      block_info = const_cast< BlockInfoSeg*>(&it->second);
      if (block_info->has_lease()
          && !block_info->ds_.empty())
      {
        block_info->ds_.push_back(ULONG_LONG_MAX);
        block_info->ds_.push_back(static_cast<uint64_t> (block_info->version_));
        block_info->ds_.push_back(static_cast<uint64_t> (block_info->lease_id_));
      }
      //block id
      iret = output.set_int32(it->first);
      if (SUCCESS != iret)
        break;
      // dataserver list
      iret = output.set_vint64(block_info->ds_);
      if (SUCCESS != iret)
        break;
      // reparse, avoid push verion&lease again when clone twice;
      BasePacket::parse_special_ds(block_info->ds_, block_info->version_, block_info->lease_id_);
    }
  }
  return iret;
}

void BatchSetBlockInfoMessage::set_read_block_ds(const uint32_t block_id, VUINT64& ds)
{
    block_infos_[block_id] = BlockInfoSeg(ds);
}

void BatchSetBlockInfoMessage::set_write_block_ds(const uint32_t block_id, VUINT64& ds,
                                                  const int32_t version, const int32_t lease)
{
    block_infos_[block_id] = BlockInfoSeg(ds, lease, version);
}

CarryBlockMessage::CarryBlockMessage()
{
  _packetHeader._pcode = CARRY_BLOCK_MESSAGE;
  expire_blocks_.clear();
  remove_blocks_.clear();
  new_blocks_.clear();
}

CarryBlockMessage::~CarryBlockMessage()
{
}

int CarryBlockMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_vint32(expire_blocks_);
  if (SUCCESS == iret)
  {
    iret = input.get_vint32(remove_blocks_);
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint32(new_blocks_);
  }
  return iret;
}

int64_t CarryBlockMessage::length() const
{
  return Serialization::get_vint32_length(expire_blocks_)
          + Serialization::get_vint32_length(remove_blocks_)
          + Serialization::get_vint32_length(new_blocks_);
}

int CarryBlockMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_vint32(expire_blocks_);
  if (SUCCESS == iret)
  {
    iret = output.set_vint32(remove_blocks_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint32(new_blocks_);
  }
  return iret;
}

void CarryBlockMessage::add_expire_id(const uint32_t block_id)
{
  expire_blocks_.push_back(block_id);
}
void CarryBlockMessage::add_new_id(const uint32_t block_id)
{
  new_blocks_.push_back(block_id);
}
void CarryBlockMessage::add_remove_id(const uint32_t block_id)
{
  remove_blocks_.push_back(block_id);
}

NewBlockMessage::NewBlockMessage()
{
  _packetHeader._pcode = NEW_BLOCK_MESSAGE;
  new_blocks_.clear();
}

NewBlockMessage::~NewBlockMessage()
{
}

int NewBlockMessage::deserialize(Stream& input)
{
  return input.get_vint32(new_blocks_);
}

int64_t NewBlockMessage::length() const
{
  return  Serialization::get_vint32_length(new_blocks_);
}

int NewBlockMessage::serialize(Stream& output)  const
{
  return output.set_vint32(new_blocks_);
}

void NewBlockMessage::add_new_id(const uint32_t block_id)
{
  new_blocks_.push_back(block_id);
}

RemoveBlockMessage::RemoveBlockMessage():
  id_(0),
  response_flag_(REMOVE_BLOCK_RESPONSE_FLAG_NO)
{
  _packetHeader._pcode = REMOVE_BLOCK_MESSAGE;
}

RemoveBlockMessage::~RemoveBlockMessage()
{
}

int RemoveBlockMessage::deserialize(Stream& input)
{
  int32_t reserve = 0;
  int32_t iret = input.get_int32(&reserve);
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*>(&id_));
  }
  if (SUCCESS == iret
      && input.get_data_length() > 0)
  {
    iret = input.get_int8(&response_flag_);
  }
  if (SUCCESS == iret
      && input.get_data_length() > 0)
  {
    iret = input.get_int64(&seqno_);
  }
  return iret;
}

int64_t RemoveBlockMessage::length() const
{
  return  INT_SIZE * 2 + INT64_SIZE + INT8_SIZE;
}

int RemoveBlockMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(0);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(id_);
  }
  if (SUCCESS == iret)
  {
    output.set_int8(response_flag_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(seqno_);
  }
  return iret;
}

RemoveBlockResponseMessage::RemoveBlockResponseMessage():
  block_id_(0)
{
  _packetHeader._pcode = REMOVE_BLOCK_RESPONSE_MESSAGE;
}

RemoveBlockResponseMessage::~RemoveBlockResponseMessage()
{

}

int RemoveBlockResponseMessage::deserialize(Stream& input)
{
  int32_t ret = input.get_int32(reinterpret_cast<int32_t*>(&block_id_));
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&seqno_);
  }
  return ret;
}

int64_t RemoveBlockResponseMessage::length() const
{
  return INT_SIZE + INT64_SIZE;
}

int RemoveBlockResponseMessage::serialize(Stream& output)  const
{
  int32_t ret = output.set_int32(block_id_);
  if (SUCCESS == ret)
  {
    output.set_int64(seqno_);
  }
  return ret;
}

ListBlockMessage::ListBlockMessage() :
  type_(0)
{
  _packetHeader._pcode = LIST_BLOCK_MESSAGE;
}

ListBlockMessage::~ListBlockMessage()
{

}

int ListBlockMessage::deserialize(Stream& input)
{
  return input.get_int32(&type_);
}

int64_t ListBlockMessage::length() const
{
  return INT_SIZE;
}

int ListBlockMessage::serialize(Stream& output)  const
{
  return output.set_int32(type_);
}

RespListBlockMessage::RespListBlockMessage() :
  status_type_(0), need_free_(0)
{
  _packetHeader._pcode = RESP_LIST_BLOCK_MESSAGE;
  blocks_.clear();
}

RespListBlockMessage::~RespListBlockMessage()
{
}

int RespListBlockMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&status_type_);
  if (SUCCESS == iret)
  {
    need_free_ = 1;
    if (status_type_ & LB_BLOCK)//blocks
    {
      iret = input.get_vint32(blocks_);
    }

    if (SUCCESS == iret
      && status_type_ & LB_PAIRS)//pairs
    {
      int32_t size = 0;
      iret = input.get_int32(&size);
      if (SUCCESS == iret)
      {
        std::vector<uint32_t> tmp;
        uint32_t block_id = 0;
        for (int32_t i = 0; i < size; ++i)
        {
          iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id));
          if (SUCCESS == iret)
          {
            tmp.clear();
            iret = input.get_vint32(tmp);
            if (SUCCESS == iret)
            {
              block_pairs_.insert(std::map<uint32_t, std::vector<uint32_t> >::value_type(block_id, tmp));
            }
            else
            {
              break;
            }
          }
          else
          {
            break;
          }
        }
      }
    }

    if (SUCCESS == iret
      && status_type_ & LB_INFOS)
    {
      int32_t size = 0;
      iret = input.get_int32(&size);
      if (SUCCESS == iret)
      {
        int64_t pos = 0;
        BlockInfo info;
        for (int32_t i = 0; i < size; ++i)
        {
          pos = 0;
          iret = info.deserialize(input.get_data(), input.get_data_length(), pos);
          if (SUCCESS == iret)
          {
            input.drain(info.length());
            block_infos_.insert(std::map<uint32_t, BlockInfo>::value_type(info.block_id_, info));
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

int64_t RespListBlockMessage::length() const
{
  int64_t len = INT_SIZE;
  // m_Blocks
  if (status_type_ & LB_BLOCK)
  {
    len += Serialization::get_vint32_length(blocks_);
  }
  // m_BlockPairs
  if (status_type_ & LB_PAIRS)
  {
    len += INT_SIZE;
    std::map<uint32_t, std::vector<uint32_t> >::const_iterator mit = block_pairs_.begin();
    for (; mit != block_pairs_.end(); mit++)
    {
      len += INT_SIZE;
      len += Serialization::get_vint32_length(mit->second);
    }
  }
  // m_BlockInfos
  if (status_type_ & LB_INFOS)
  {
    len += INT_SIZE;
    BlockInfo info;
    len += block_infos_.size() * info.length();
  }
  return len;
}

int RespListBlockMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(status_type_);
  if (SUCCESS == iret)
  {
    if (status_type_ & LB_BLOCK)
    {
      iret = output.set_vint32(blocks_);
    }

    if (SUCCESS == iret
      && status_type_ & LB_PAIRS)
    {
      iret = output.set_int32(block_pairs_.size());
      if (SUCCESS == iret)
      {
        std::map<uint32_t, std::vector<uint32_t> >:: const_iterator iter =
          block_pairs_.begin();
        for (; iter != block_pairs_.end(); ++iter)
        {
          iret = output.set_int32(iter->first);
          if (SUCCESS != iret)
            break;
          iret = output.set_vint32(iter->second);
          if (SUCCESS != iret)
            break;
        }
      }
    }

    if (SUCCESS == iret
        && status_type_ & LB_INFOS)
    {
      iret = output.set_int32(block_infos_.size());
      if (SUCCESS == iret)
      {
        std::map<uint32_t, BlockInfo>::const_iterator iter =
          block_infos_.begin();
        for (; iter != block_infos_.end(); ++iter)
        {
          int64_t pos = 0;
          iret = const_cast<BlockInfo*>(&(iter->second))->serialize(output.get_free(), output.get_free_length(), pos);
          if (SUCCESS == iret)
            output.pour(iter->second.length());
          else
            break;
        }
      }
    }
  }
  return iret;
}

void RespListBlockMessage::add_block_id(const uint32_t block_id)
{
  blocks_.push_back(block_id);
}

UpdateBlockInfoMessage::UpdateBlockInfoMessage() :
  block_id_(0), server_id_(0), repair_(0)
{
  _packetHeader._pcode = UPDATE_BLOCK_INFO_MESSAGE;
  memset(&db_stat_, 0, sizeof(db_stat_));
}

UpdateBlockInfoMessage::~UpdateBlockInfoMessage()
{

}

int UpdateBlockInfoMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int64(reinterpret_cast<int64_t*> (&server_id_));
  }
  int32_t have_block = 0;
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&have_block);
  }
  if (SUCCESS == iret
    && have_block)
  {
    int64_t pos = 0;
    iret = block_info_.deserialize(input.get_data(), input.get_data_length(), pos);
    if (SUCCESS == iret)
    {
      input.drain(block_info_.length());
    }
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&repair_);
  }
  int32_t have_sdbm = 0;
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&have_sdbm);
  }
  if (SUCCESS == iret
      && have_sdbm > 0)
  {
    int64_t pos = 0;
    iret = db_stat_.deserialize(input.get_data(), input.get_data_length(), pos);
    if (SUCCESS == iret)
    {
      input.drain(db_stat_.length());
    }
  }
  return iret;
}

int64_t UpdateBlockInfoMessage::length() const
{
  int64_t len = INT64_SIZE + INT_SIZE * 4;
  if (block_info_ .block_id_ > 0)
  {
    len += block_info_.length();
  }
  if (db_stat_.item_count_ > 0)
  {
    len += db_stat_.length();
  }
  return len;
}

int UpdateBlockInfoMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int64(server_id_);
  }
  int32_t have_block = block_info_.block_id_ > 0 ? 1 : 0;
  if (SUCCESS == iret)
  {
    iret = output.set_int32(have_block);
  }
  int64_t pos = 0;
  if (SUCCESS == iret
    && have_block)
  {
    iret = block_info_.serialize(output.get_free(), output.get_free_length(), pos);
    if (SUCCESS == iret)
    {
      output.pour(block_info_.length());
    }
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(repair_);
  }
  int32_t have_sdbm = db_stat_.item_count_ > 0 ? 0 : 1;
  if (SUCCESS == iret)
  {
    iret = output.set_int32(have_sdbm);
  }
  if (SUCCESS == iret
    && have_sdbm)
  {
    pos = 0;
    iret = db_stat_.serialize(output.get_free(), output.get_free_length(), pos);
    if (SUCCESS == iret)
    {
      output.pour(db_stat_.length());
    }
  }
  return iret;
}

ResetBlockVersionMessage::ResetBlockVersionMessage() :
  block_id_(0)
{
  _packetHeader._pcode = RESET_BLOCK_VERSION_MESSAGE;
}

ResetBlockVersionMessage::~ResetBlockVersionMessage()
{
}

int ResetBlockVersionMessage::deserialize(Stream& input)
{
  return input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
}

int64_t ResetBlockVersionMessage::length() const
{
  return INT_SIZE;
}

int ResetBlockVersionMessage::serialize(Stream& output)  const
{
  return output.set_int32(block_id_);
}

BlockFileInfoMessage::BlockFileInfoMessage() :
  block_id_(0)
{
  _packetHeader._pcode = BLOCK_FILE_INFO_MESSAGE;
  fileinfo_list_.clear();
}

BlockFileInfoMessage::~BlockFileInfoMessage()
{
}

int BlockFileInfoMessage::deserialize(Stream& input)
{
  int32_t size = 0;
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&size);
  }
  if (SUCCESS == iret)
  {
    FileInfo info;
    for (int32_t i = 0; i < size; ++i)
    {
      int64_t pos = 0;
      iret = info.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        input.drain(info.length());
        fileinfo_list_.push_back(info);
      }
      else
      {
        break;
      }
    }
  }
  return iret;
}

int64_t BlockFileInfoMessage::length() const
{
  FileInfo info;
  return INT_SIZE * 2 + fileinfo_list_.size() * info.length();
}

int BlockFileInfoMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(fileinfo_list_.size());
  }
  if (SUCCESS == iret)
  {
    FILE_INFO_LIST::const_iterator iter = fileinfo_list_.begin();
    for (; iter != fileinfo_list_.end(); ++iter)
    {
      int64_t pos = 0;
      iret = (*iter).serialize(output.get_free(), output.get_free_length(), pos);
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
  return iret;
}

BlockRawMetaMessage::BlockRawMetaMessage() :
  block_id_(0)
{
  _packetHeader._pcode = BLOCK_RAW_META_MESSAGE;
  raw_meta_list_.clear();
}

BlockRawMetaMessage::~BlockRawMetaMessage()
{
}

int BlockRawMetaMessage::deserialize(Stream& input)
{
  int32_t size = 0;
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int32(&size);
  }
  if (SUCCESS == iret)
  {
    RawMeta raw;
    for (int32_t i = 0; i < size; ++i)
    {
      int64_t pos = 0;
      iret = raw.deserialize(input.get_data(), input.get_data_length(), pos);
      if (SUCCESS == iret)
      {
        input.drain(raw.length());
        raw_meta_list_.push_back(raw);
      }
      else
      {
        break;
      }
    }
  }
  return iret;
}

int64_t BlockRawMetaMessage::length() const
{
  RawMeta raw;
  return  INT_SIZE * 2 + raw_meta_list_.size() * raw.length();
}

int BlockRawMetaMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(raw_meta_list_.size());
  }
  if (SUCCESS == iret)
  {
    RawMetaVec::const_iterator iter = raw_meta_list_.begin();
    for (; iter != raw_meta_list_.end(); ++iter)
    {
      int64_t pos = 0;
      iret = const_cast<RawMeta*>(&(*iter))->serialize(output.get_free(), output.get_free_length(), pos);
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
  return iret;
}

BlockWriteCompleteMessage::BlockWriteCompleteMessage() :
  server_id_(0), write_complete_status_(WRITE_COMPLETE_STATUS_NO), unlink_flag_(UNLINK_FLAG_NO)
{
  _packetHeader._pcode = BLOCK_WRITE_COMPLETE_MESSAGE;
}

BlockWriteCompleteMessage::~BlockWriteCompleteMessage()
{
}

int BlockWriteCompleteMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int64( reinterpret_cast<int64_t*> (&server_id_));
  if (SUCCESS == iret)
  {
    int64_t pos = 0;
    iret = block_info_.deserialize(input.get_data(), input.get_data_length(), pos);
    if (SUCCESS == iret)
    {
      input.drain(block_info_.length());
    }
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*> (&lease_id_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*> (&write_complete_status_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*> (&unlink_flag_));
  }
  return iret;
}

int64_t BlockWriteCompleteMessage::length() const
{
  return INT64_SIZE + block_info_.length() + INT_SIZE * 3;
}

int BlockWriteCompleteMessage::serialize(Stream& output)  const
{
  int32_t iret = block_info_.block_id_ <= 0 ? ERROR : SUCCESS;
  if (SUCCESS == iret)
  {
    iret = output.set_int64(server_id_);
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
    iret = output.set_int32(lease_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(write_complete_status_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(unlink_flag_);
  }
  return iret;
}

ListBitMapMessage::ListBitMapMessage() :
  type_(0)
{
  _packetHeader._pcode = LIST_BITMAP_MESSAGE;
}

ListBitMapMessage::~ListBitMapMessage()
{
}

int ListBitMapMessage::deserialize(Stream& input)
{
  return input.get_int32(&type_);
}

int64_t ListBitMapMessage::length() const
{
  return INT_SIZE;
}

int ListBitMapMessage::serialize(Stream& output)  const
{
  return output.set_int32(type_);
}

RespListBitMapMessage::RespListBitMapMessage() :
  ubitmap_len_(0), uuse_len_(0), data_(NULL), alloc_(false)
{
  _packetHeader._pcode = RESP_LIST_BITMAP_MESSAGE;
}

RespListBitMapMessage::~RespListBitMapMessage()
{
  if ((NULL != data_ ) && (alloc_ ))
  {
    ::free(data_);
    data_ = NULL;
  }
}

char* RespListBitMapMessage::alloc_data(const int32_t len)
{
  if (len <= 0)
  {
    return NULL;
  }
  if (data_ != NULL)
  {
    ::free(data_);
    data_ = NULL;
  }
  ubitmap_len_ = len;
  data_ = (char*) malloc(len);
  alloc_ = true;
  return data_;
}

int RespListBitMapMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&uuse_len_));
  if (SUCCESS == iret)
  {
    input.get_int32(reinterpret_cast<int32_t*> (&ubitmap_len_));
  }
  if (SUCCESS == iret
    && ubitmap_len_ > 0)
  {
    char* data = alloc_data(ubitmap_len_);
    iret = NULL != data ? SUCCESS : ERROR;
    if (SUCCESS == iret)
    {
      iret = input.get_bytes(data, ubitmap_len_);
    }
  }
  return iret;
}

int64_t RespListBitMapMessage::length() const
{
  int64_t len = INT_SIZE * 2;
  if (ubitmap_len_ > 0)
  {
    len += ubitmap_len_;
  }
  return len;
}

int RespListBitMapMessage::serialize(Stream& output)  const
{
  int32_t iret = output.set_int32(uuse_len_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(ubitmap_len_);
  }
  if (SUCCESS == iret
      && ubitmap_len_ > 0)
  {
    iret = output.set_bytes(data_, ubitmap_len_);
  }
  return iret;
}
    
} //namespace dfs
} //namespace neptune
