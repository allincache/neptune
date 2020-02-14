#include "crc_error_message.h"

namespace neptune {
namespace dfs {

CrcErrorMessage::CrcErrorMessage() :
  block_id_(0), file_id_(0), crc_(0), error_flag_(CRC_DS_PATIAL_ERROR)
{
  _packetHeader._pcode = CRC_ERROR_MESSAGE;
}

CrcErrorMessage::~CrcErrorMessage()
{
}

int CrcErrorMessage::deserialize(Stream& input)
{
  int32_t iret = input.get_int32(reinterpret_cast<int32_t*>(&block_id_));
  if (SUCCESS == iret)
  {
    iret = input.get_int64(reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*>(&crc_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_int32(reinterpret_cast<int32_t*>(&error_flag_));
  }
  if (SUCCESS == iret)
  {
    iret = input.get_vint64(fail_server_);
  }
  return iret;
}

int64_t CrcErrorMessage::length() const
{
  return INT_SIZE * 3 + INT64_SIZE + Serialization::get_vint64_length(fail_server_);
}

int CrcErrorMessage::serialize(Stream& output) const 
{
  int32_t iret = output.set_int32(block_id_);
  if (SUCCESS == iret)
  {
    iret = output.set_int64(file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(crc_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_int32(error_flag_);
  }
  if (SUCCESS == iret)
  {
    iret = output.set_vint64(fail_server_);
  }
  return iret;
}

} //namespace dfs
} //namespace neptune
