#include "metaserver_client_message.h"


namespace neptune {
namespace dfs {

BaseMetaParameter::BaseMetaParameter() :
  app_id_(0), user_id_(0),
  version_(INVALID_TABLE_VERSION),
  file_path_()
{
}
BaseMetaParameter::~BaseMetaParameter()
{
}
int BaseMetaParameter::deserialize(Stream& input)
{
  int ret = ERROR;

  ret = input.get_int64(&app_id_);
  LOG(DEBUG, "app_id: %ld", app_id_);
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&user_id_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_string(file_path_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&version_);
  }
  return ret;
}
int64_t BaseMetaParameter::length() const
{
  return 2 * INT64_SIZE + Serialization::get_string_length(file_path_);
}
int BaseMetaParameter::serialize(Stream& output) const
{
  int ret = SUCCESS;
  ret = output.set_int64(app_id_);
  if (SUCCESS == ret)
  {
    ret = output.set_int64(user_id_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_string(file_path_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(version_);
  }
  return ret;
}

FilepathActionMessage::FilepathActionMessage() :
  BaseMetaParameter(), new_file_path_(), action_(NON_ACTION)
{
  _packetHeader._pcode = FILEPATH_ACTION_MESSAGE;
}

FilepathActionMessage::~FilepathActionMessage()
{

}

int FilepathActionMessage::deserialize(Stream& input)
{
  int ret = ERROR;

  ret = BaseMetaParameter::deserialize(input);
  if (SUCCESS == ret)
  {
    ret = input.get_string(new_file_path_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int8(reinterpret_cast<int8_t*>(&action_));
  }
  return ret;
}

int64_t FilepathActionMessage::length() const
{
  return BaseMetaParameter::length() + Serialization::get_string_length(new_file_path_) + INT8_SIZE;
}

int FilepathActionMessage::serialize(Stream& output) const
{
  int ret = SUCCESS;
  ret = BaseMetaParameter::serialize(output);
  if (SUCCESS == ret)
  {
    ret = output.set_string(new_file_path_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int8(action_);
  }
  return ret;
}

WriteFilepathMessage::WriteFilepathMessage() :
  BaseMetaParameter()
{
  _packetHeader._pcode = WRITE_FILEPATH_MESSAGE;
}

WriteFilepathMessage::~WriteFilepathMessage()
{

}

int WriteFilepathMessage::deserialize(Stream& input)
{
  int ret = ERROR;

  ret = BaseMetaParameter::deserialize(input);
  if (SUCCESS == ret)
  {
    ret = frag_info_.deserialize(input);
  }
  return ret;
}

int64_t WriteFilepathMessage::length() const
{
  return BaseMetaParameter::length() + frag_info_.get_length();
}

int WriteFilepathMessage::serialize(Stream& output) const
{
  int ret = SUCCESS;
  ret = BaseMetaParameter::serialize(output);
  if (SUCCESS == ret)
  {
    ret = frag_info_.serialize(output);
  }
  return ret;
}


ReadFilepathMessage::ReadFilepathMessage() :
  BaseMetaParameter(), offset_(0), size_(0)
{
  _packetHeader._pcode = READ_FILEPATH_MESSAGE;
}

ReadFilepathMessage::~ReadFilepathMessage()
{

}

int ReadFilepathMessage::deserialize(Stream& input)
{
  int ret = ERROR;

  ret = BaseMetaParameter::deserialize(input);
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&offset_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&size_);
  }
  return ret;
}

int64_t ReadFilepathMessage::length() const
{
  return BaseMetaParameter::length() + 2 * INT64_SIZE;
}

int ReadFilepathMessage::serialize(Stream& output) const
{
  int ret = SUCCESS;
  ret = BaseMetaParameter::serialize(output);
  if (SUCCESS == ret)
  {
    ret = output.set_int64(offset_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(size_);
  }
  return ret;
}

RespReadFilepathMessage::RespReadFilepathMessage() :
  still_have_(0)
{
  _packetHeader._pcode = RESP_READ_FILEPATH_MESSAGE;
}

RespReadFilepathMessage::~RespReadFilepathMessage()
{

}

int RespReadFilepathMessage::deserialize(Stream& input)
{
  int ret = ERROR;

  ret = input.get_int8(reinterpret_cast<int8_t*>(&still_have_));

  if (SUCCESS == ret)
  {
    ret = frag_info_.deserialize(input);
  }
  return ret;
}

int64_t RespReadFilepathMessage::length() const
{
  return INT8_SIZE + frag_info_.get_length();
}

int RespReadFilepathMessage::serialize(Stream& output) const
{
  int ret = SUCCESS;
  ret = output.set_int8(still_have_);
  if (SUCCESS == ret)
  {
    ret = frag_info_.serialize(output);
  }
  return ret;
}

LsFilepathMessage::LsFilepathMessage() :
  BaseMetaParameter(), file_type_(NORMAL_FILE)
{
  _packetHeader._pcode = LS_FILEPATH_MESSAGE;
}

LsFilepathMessage::~LsFilepathMessage()
{
}

int64_t LsFilepathMessage::length() const
{
  return BaseMetaParameter::length() + INT64_SIZE + INT8_SIZE;
}

int LsFilepathMessage::serialize(Stream& output) const
{
  int ret = SUCCESS;
  ret = output.set_int64(app_id_);
  if (SUCCESS == ret)
  {
    ret = output.set_int64(user_id_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(pid_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_string(file_path_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int8(static_cast<char>(file_type_));
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(version_);
  }
  return ret;
}

int LsFilepathMessage::deserialize(Stream& input)
{
  int ret = input.get_int64(&app_id_);
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&user_id_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&pid_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_string(file_path_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int8(reinterpret_cast<int8_t*>(&file_type_));
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&version_);
  }
  return ret;
}

RespLsFilepathMessage::RespLsFilepathMessage() :
  still_have_(false),
  version_(INVALID_TABLE_VERSION)
{
  meta_infos_.clear();
  _packetHeader._pcode = RESP_LS_FILEPATH_MESSAGE;
}

RespLsFilepathMessage::~RespLsFilepathMessage()
{

}
int64_t RespLsFilepathMessage::length() const
{
  int64_t len = INT8_SIZE + INT_SIZE;
  for (std::vector<MetaInfo>::const_iterator it = meta_infos_.begin();
        it != meta_infos_.end(); it++)
  {
    len += it->length();
  }
  return len;
}

int RespLsFilepathMessage::serialize(Stream& output) const
{
  int ret = output.set_int8(still_have_);
  if (SUCCESS == ret)
  {
    ret = output.set_int32(meta_infos_.size());
  }
  if (SUCCESS == ret)
  {
    for (std::vector<MetaInfo>::const_iterator it = meta_infos_.begin();
          SUCCESS == ret && it != meta_infos_.end(); it++)
    {
      ret = it->serialize(output);
    }
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(version_);
  }
  return ret;
}

int RespLsFilepathMessage::deserialize(Stream& input)
{
  int ret = input.get_int8(reinterpret_cast<int8_t*>(&still_have_));
  int32_t count = 0;
  if (SUCCESS == ret)
  {
    ret = input.get_int32(&count);
  }
  if (SUCCESS == ret)
  {
    for (int32_t i = 0; i < count; i++)
    {
      MetaInfo file_meta_info;
      ret = file_meta_info.deserialize(input);
      if (ret != SUCCESS)
      {
        break;
      }
      meta_infos_.push_back(file_meta_info);
    }
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&version_);
  }
  return ret;
}

} //namespace dfs
} //namespace neptune
