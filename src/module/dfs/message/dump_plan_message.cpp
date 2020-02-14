#include "dump_plan_message.h"

namespace neptune {
namespace dfs {

DumpPlanMessage::DumpPlanMessage():
  reserve_(0)
{
  _packetHeader._pcode= DUMP_PLAN_MESSAGE;
}

DumpPlanMessage::~DumpPlanMessage()
{

}

int DumpPlanMessage::deserialize(Stream& input)
{
  return input.get_int8(&reserve_);
}

int64_t DumpPlanMessage::length() const
{
  return INT8_SIZE;
}

int DumpPlanMessage::serialize(Stream& output) const
{
  return output.set_int8(reserve_);
}

DumpPlanResponseMessage::DumpPlanResponseMessage()
{
  _packetHeader._pcode= DUMP_PLAN_RESPONSE_MESSAGE;
}

DumpPlanResponseMessage::~DumpPlanResponseMessage()
{
}

int DumpPlanResponseMessage::deserialize(Stream& input)
{
  int32_t iret = SUCCESS;
  if (input.get_data_length() > 0)
  {
    data_.writeBytes(input.get_data(), input.get_data_length());
  }
  return iret;
}

int64_t DumpPlanResponseMessage::length() const
{
  return data_.getDataLen();
}

int DumpPlanResponseMessage::serialize(Stream& output) const
{
  int32_t iret = SUCCESS;
  if (data_.getDataLen() > 0)
  {
    iret = output.set_bytes(data_.getData(), data_.getDataLen());
  }
  return iret;
}

} //namespace dfs
} //namespace neptune
