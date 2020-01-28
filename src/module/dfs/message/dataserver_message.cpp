#include "dataserver_message.h"

namespace neptune {
namespace dfs {

SetDataserverMessage::SetDataserverMessage() :
  has_block_(HAS_BLOCK_FLAG_NO)
{
  _packetHeader._pcode = SET_DATASERVER_MESSAGE;
  memset(&ds_, 0, sizeof(ds_));
  blocks_.clear();
}

SetDataserverMessage::~SetDataserverMessage()
{

}

int SetDataserverMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = ds_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.drain(ds_.length());
    iret = input.get_int32(reinterpret_cast<int32_t*> (&has_block_));
    if (SUCCESS == iret)
    {
      if (has_block_  == HAS_BLOCK_FLAG_YES)
      {
        int32_t size = 0;
        iret = input.get_int32(&size);
        if (SUCCESS == iret)
        {
          BlockInfo info;
          for (int32_t i = 0; i < size; ++i)
          {
            pos  = 0;
            iret = info.deserialize(input.get_data(), input.get_data_length(), pos);
            if (SUCCESS == iret)
            {
              input.drain(info.length());
              blocks_.push_back(info);
            }
            else
            {
              break;
            }
          }
        }
      }
    }
  }
  return iret;
}

int64_t SetDataserverMessage::length() const
{
  int64_t len = ds_.length() + INT_SIZE;
  if (has_block_ > 0)
  {
    len += INT_SIZE;
    BlockInfo info;
    len += blocks_.size() * info.length();
  }
  return len;
}

int SetDataserverMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = ds_.id_ <= 0 ? ERROR : SUCCESS;
  if (SUCCESS == iret)
  {
    iret = ds_.serialize(output.get_free(), output.get_free_length(), pos);
    if (SUCCESS == iret)
    {
      output.pour(ds_.length());
      iret = output.set_int32(has_block_);
    }
  }
  if (SUCCESS == iret)
  {
    if (has_block_ == HAS_BLOCK_FLAG_YES)
    {
      iret = output.set_int32(blocks_.size());
      if (SUCCESS == iret)
      {
        std::vector<BlockInfo>::const_iterator iter = blocks_.begin();
        for (; iter != blocks_.end(); ++iter)
        {
          pos = 0;
          iret = const_cast<BlockInfo*>((&(*iter)))->serialize(output.get_free(), output.get_free_length(), pos);
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

void SetDataserverMessage::set_ds(DataServerStatInfo* ds)
{
  if (NULL != ds)
    ds_ = *ds;
}

void SetDataserverMessage::add_block(BlockInfo* block_info)
{
  if (NULL != block_info)
    blocks_.push_back(*block_info);
}

CallDsReportBlockRequestMessage::CallDsReportBlockRequestMessage():
  server_(0)
{
  _packetHeader._pcode = REQ_CALL_DS_REPORT_BLOCK_MESSAGE;
}

CallDsReportBlockRequestMessage::~CallDsReportBlockRequestMessage()
{

}

int CallDsReportBlockRequestMessage::deserialize(Stream& input)
{
  return input.get_int64(reinterpret_cast<int64_t*>(&server_));
}

int64_t CallDsReportBlockRequestMessage::length() const
{
  return INT64_SIZE;
}

int CallDsReportBlockRequestMessage::serialize(Stream& output) const
{
  return output.set_int64(server_);
}

ReportBlocksToNsRequestMessage::ReportBlocksToNsRequestMessage():
  server_(0)
{
  _packetHeader._pcode = REQ_REPORT_BLOCKS_TO_NS_MESSAGE;
  blocks_.clear();
}

ReportBlocksToNsRequestMessage::~ReportBlocksToNsRequestMessage()
{

}

int ReportBlocksToNsRequestMessage::deserialize(Stream& input)
{
  int32_t iret =input.get_int64(reinterpret_cast<int64_t*>(&server_));
  if (SUCCESS == iret)
  {
    int32_t size = 0;
    int64_t pos = 0;
    iret = input.get_int32(&size);
    if (SUCCESS == iret)
    {
      BlockInfo info;
      for (int32_t i = 0; i < size; ++i)
      {
        pos  = 0;
        iret = info.deserialize(input.get_data(), input.get_data_length(), pos);
        if (SUCCESS == iret)
        {
          input.drain(info.length());
          blocks_.insert(info);
        }
        else
        {
          break;
        }
      }
    }
  }
  return iret;
}

int64_t ReportBlocksToNsRequestMessage::length() const
{
  BlockInfo info;
  return INT64_SIZE + INT_SIZE + blocks_.size() * info.length();
}

int ReportBlocksToNsRequestMessage::serialize(Stream& output) const
{
  int64_t pos = 0;
  int32_t iret = server_ <= 0 ? ERROR : SUCCESS;
  if (SUCCESS == iret)
  {
    iret = output.set_int64(server_);
    if (SUCCESS == iret)
    {
      iret = output.set_int32(blocks_.size());
    }
  }
  if (SUCCESS == iret)
  {
    std::set<BlockInfo>::const_iterator iter = blocks_.begin();
    for (; iter != blocks_.end(); ++iter)
    {
      pos = 0;
      iret = const_cast<BlockInfo*>((&(*iter)))->serialize(output.get_free(), output.get_free_length(), pos);
      if (SUCCESS == iret)
        output.pour((*iter).length());
      else
        break;
    }
  }
  return iret;
}

ReportBlocksToNsResponseMessage::ReportBlocksToNsResponseMessage():
server_(0),
status_(0)
{
  _packetHeader._pcode = RSP_REPORT_BLOCKS_TO_NS_MESSAGE;
}

ReportBlocksToNsResponseMessage::~ReportBlocksToNsResponseMessage()
{

}

int ReportBlocksToNsResponseMessage::deserialize(Stream& input)
{
  int32_t iret =input.get_int64(reinterpret_cast<int64_t*>(&server_));
  if (SUCCESS == iret)
  {
    iret = input.get_int8(&status_);
  }

  if (SUCCESS == iret)
  {
    int32_t size = 0;
    iret = input.get_int32(&size);
    if (SUCCESS == iret)
    {
      uint32_t id = 0;
      for (int32_t i = 0; i < size; ++i)
      {
        iret = input.get_int32(reinterpret_cast<int32_t*>(&id));
        if (SUCCESS != iret)
          break;
      }
    }
  }
  return iret;
}

int64_t ReportBlocksToNsResponseMessage::length() const
{
  return INT64_SIZE + INT8_SIZE + Serialization::get_vint32_length(expire_blocks_);
}

int ReportBlocksToNsResponseMessage::serialize(Stream& output) const
{
  int32_t iret = server_ == 0 ? ERROR : SUCCESS;
  if (SUCCESS == iret)
  {
    iret = output.set_int64(server_);
    if (SUCCESS == iret)
    {
      iret = output.set_int8(status_);
    }
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(expire_blocks_.size());
    if (SUCCESS == iret)
    {
      std::vector<uint32_t>::const_iterator iter = expire_blocks_.begin();
      for (; iter != expire_blocks_.end(); ++iter)
      {
        iret = output.set_int32((*iter));
        if (SUCCESS != iret)
          break;
      }
    }
  }
  return iret;
}

} //namespace dfs
} //namespace neptune
