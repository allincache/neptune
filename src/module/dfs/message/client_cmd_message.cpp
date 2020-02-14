#include "client_cmd_message.h"

namespace neptune {
namespace dfs {

ClientCmdMessage::ClientCmdMessage()
{
  _packetHeader._pcode = CLIENT_CMD_MESSAGE;
  memset(&info_, 0, sizeof(ClientCmdInformation));
}

ClientCmdMessage::~ClientCmdMessage()
{
}

int ClientCmdMessage::deserialize(Stream& input)
{
  int64_t pos = 0;
  int32_t iret = info_.deserialize(input.get_data(), input.get_data_length(), pos);
  if (SUCCESS == iret)
  {
    input.pour(info_.length());
  }
  return iret;
}

int64_t ClientCmdMessage::length() const
{
  return info_.length();
}

int ClientCmdMessage::serialize(Stream& output) const 
{
  int64_t pos = 0;
  int32_t iret = info_.serialize(output.get_free(), output.get_free_length(), pos);
  if (SUCCESS == iret)
  {
    output.pour(info_.length());
  }
  return iret;
}

} //namespace dfs
} //namespace neptune
