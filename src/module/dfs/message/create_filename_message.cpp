#include "create_filename_message.h"

namespace neptune {
namespace dfs {

CreateFilenameMessage::CreateFilenameMessage() :
  block_id_(0), file_id_(0)
{
  _packetHeader._pcode = CREATE_FILENAME_MESSAGE;
}

CreateFilenameMessage::~CreateFilenameMessage()
{
}

int CreateFilenameMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int64(reinterpret_cast<int64_t*> (&file_id_));
  }
  return iret;
}

int64_t CreateFilenameMessage::length() const
{
  return INT_SIZE + INT64_SIZE;
}

int CreateFilenameMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int64(file_id_);
  }
  return iret;
}

RespCreateFilenameMessage::RespCreateFilenameMessage() :
  block_id_(0), file_id_(0), file_number_(0)
{
  _packetHeader._pcode = RESP_CREATE_FILENAME_MESSAGE;
}

RespCreateFilenameMessage::~RespCreateFilenameMessage()
{
}

int RespCreateFilenameMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*> (&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int64(reinterpret_cast<int64_t*> (&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int64(reinterpret_cast<int64_t*> (&file_number_));
  }
  return iret;
}

int64_t RespCreateFilenameMessage::length() const
{
  return INT_SIZE + INT64_SIZE * 2;
}

int RespCreateFilenameMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int64(file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int64(file_number_);
  }
  return iret;
}

} //namespace dfs
} //namespace neptune
