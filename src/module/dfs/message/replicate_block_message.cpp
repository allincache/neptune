#include "replicate_block_message.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

ReplicateBlockMessage::ReplicateBlockMessage() :
  command_(0), expire_(0)
{
  _packetHeader._pcode = REPLICATE_BLOCK_MESSAGE;
  memset(&repl_block_, 0, sizeof(ReplBlock));
}

ReplicateBlockMessage::~ReplicateBlockMessage()
{
}

int ReplicateBlockMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(&command_);
  if (SUCCESS == iret)
  {
    iret =  input.get_int32(&expire_);
  }
  if (SUCCESS == iret)
  {
    int64_t pos = 0;
    iret = repl_block_.deserialize(input.get_data(), input.get_data_length(), pos);
    if (SUCCESS == iret)
    {
      input.drain(repl_block_.length());
    }
  }
  if (SUCCESS == iret
    && input.get_data_length() > 0 )
  {
    iret = input.get_int64(&seqno_);
  }
  return iret;
}

int ReplicateBlockMessage::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = Serialization::get_int32(data, data_len, pos, &command_);
  if (SUCCESS == iret)
  {
    iret =  Serialization::get_int32(data, data_len, pos, &expire_);
  }
  if (SUCCESS == iret)
  {
    iret = repl_block_.deserialize(data, data_len, pos);
  }
  if (SUCCESS == iret
    && pos + INT64_SIZE <= data_len)
  {
    iret = Serialization::get_int64(data, data_len, pos, &seqno_);
  }
  return iret;
}

int64_t ReplicateBlockMessage::length() const
{
  return INT_SIZE * 2 + INT64_SIZE + repl_block_.length();
}

int ReplicateBlockMessage::serialize(Stream& output) const
{
  int32_t iret = output.set_int32(command_);
  if (SUCCESS == iret)
  {
    iret = output.set_int32(expire_);
  }
  if (SUCCESS == iret)
  {
    int64_t pos = 0;
    iret = repl_block_.serialize(output.get_free(), output.get_free_length(), pos);
    if (SUCCESS == iret)
    {
      output.pour(repl_block_.length());
    }
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(seqno_);
  }
  return iret;
}

void ReplicateBlockMessage::dump(void) const
{
  //LOG(INFO, "seqno: %"PRI64_PREFIX"d, command: %d, expire: %d, block: %u, source: %s, target: %s, start_time: %d, is_move: %s, server_count: %d",
  //  seqno_, command_, expire_, repl_block_.block_id_,
  //  CNetUtil::addrToString(repl_block_.source_id_).c_str(),
  //  CNetUtil::addrToString(repl_block_.destination_id_).c_str(),
  //  repl_block_.start_time_,
  //  repl_block_.is_move_ == REPLICATE_BLOCK_MOVE_FLAG_NO ? "replicate" : "move", repl_block_.server_count_);
}


} //namespace dfs
} //namespace neptune
