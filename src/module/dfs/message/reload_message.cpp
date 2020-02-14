#include "reload_message.h"

namespace neptune {
namespace dfs {

ReloadConfigMessage::ReloadConfigMessage() :
  flag_(0)
{
  _packetHeader._pcode = RELOAD_CONFIG_MESSAGE;
}

ReloadConfigMessage::~ReloadConfigMessage()
{

}

int ReloadConfigMessage::deserialize(Stream& input)
{
  return input.get_int32(&flag_);
}

int64_t ReloadConfigMessage::length() const
{
  return INT_SIZE;
}

int ReloadConfigMessage::serialize(Stream& output) const
{
  return output.set_int32(flag_);
}

void ReloadConfigMessage::set_switch_cluster_flag(const int32_t flag)
{
  flag_ = flag;
}

int32_t ReloadConfigMessage::get_switch_cluster_flag() const
{
  return flag_;
}

} //namespace dfs
} //namespace neptune
