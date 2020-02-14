#include "Internal.h"
#include "Serialization.h"
#include "base/container/Vector.h"

using namespace std;

namespace neptune {
namespace base {
  
int FileInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, usize_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, modify_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, create_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, flag_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, crc_);
  }
  return iret;
}

int FileInfo::deserialize(const char*data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &usize_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &modify_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &create_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &flag_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&crc_));
  }
  return iret;
}

int64_t FileInfo::length() const
{
  return INT64_SIZE  + INT_SIZE  * 7;
}

int SSMScanParameter::serialize(char* data, const int64_t data_len, int64_t& pos ) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, addition_param1_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, addition_param2_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, start_next_position_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, should_actual_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int16(data, data_len, pos, child_type_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int8(data, data_len, pos, type_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int8(data, data_len, pos, end_flag_);
  }
  // if (SUCCESS == iret)
  // {
  //   iret = Serialization::set_int32(data, data_len, pos, data_.getDataLen());
  
  // if (SUCCESS == iret)
  // {
  //   if (data_.getDataLen() > 0)
  //   {
  //     iret = Serialization::set_bytes(data, data_len, pos, data_.getData(), data_.getDataLen());
  //   }
  
  return iret;
}

int SSMScanParameter::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&addition_param1_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&addition_param2_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&start_next_position_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&should_actual_count_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int16(data, data_len, pos, &child_type_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int8(data, data_len, pos, &type_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int8(data, data_len, pos, &end_flag_);
  }
  int32_t len = 0;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &len);
  }
  if (SUCCESS == iret)
  {
    if (len > 0)
    {
      //data_.ensureFree(len);
      //data_.pourData(len);
      //iret = Serialization::get_bytes(data, data_len, pos, data_.getData(), len);
    }
  }
  return iret;
}

int64_t SSMScanParameter::length() const
{
  return INT_SIZE * 6; //+ data_.getDataLen();
}

int BlockInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, version_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, file_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, del_file_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, del_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, seq_no_);
  }
  return iret;
}

int BlockInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &version_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &file_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &del_file_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &del_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&seq_no_));
  }
  return iret;
}

int64_t BlockInfo::length() const
{
  return INT_SIZE * 7;
}
int RawMeta::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{

  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&fileid_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &location_.inner_offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &location_.size_);
  }
  return iret;
}
int RawMeta::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, fileid_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, location_.inner_offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, location_.size_);
  }
  return iret;
}
int64_t RawMeta::length() const
{
  return INT64_SIZE + INT_SIZE * 2;
}

int ReplBlock::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, source_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, destination_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, start_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, is_move_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, server_count_);
  }
  return iret;
}

int ReplBlock::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&source_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&destination_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &start_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &is_move_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &server_count_);
  }
  return iret;
}

int64_t ReplBlock::length() const
{
  return  INT_SIZE * 4 + INT64_SIZE * 2;
}

int CheckBlockInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = (NULL != data && data_len - pos >= length()) ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, version_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, file_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, total_size_);
  }
  return iret;
}

int CheckBlockInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = (NULL != data && data_len - pos >= length()) ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &version_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&file_count_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&total_size_));
  }
  return iret;
}

int64_t CheckBlockInfo::length() const
{
  return  INT_SIZE * 4;
}

int Throughput::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&write_byte_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&write_file_count_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&read_byte_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&read_file_count_));
  }
  return iret;
}
int Throughput::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, write_byte_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, write_file_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, read_byte_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, read_file_count_);
  }
  return iret;
}

int64_t Throughput::length() const
{
  return INT64_SIZE * 4;
}
int DataServerStatInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, &use_capacity_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, &total_capacity_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &current_load_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &last_update_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &startup_time_);
  }
  if (SUCCESS == iret)
  {
    iret = total_tp_.deserialize(data, data_len, pos);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &current_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&status_));
  }
  return iret;
}
int DataServerStatInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, use_capacity_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, total_capacity_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, current_load_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, last_update_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, startup_time_);
  }
  if (SUCCESS == iret)
  {
    iret = total_tp_.serialize(data, data_len, pos);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, current_time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, status_);
  }
  return iret;
}
int64_t DataServerStatInfo::length() const
{
  return  INT64_SIZE * 3 + total_tp_.length() + INT_SIZE * 6;
}
int WriteDataInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &length_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&is_server_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_number_));
  }
  return iret;
}
int WriteDataInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, length_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, is_server_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_number_);
  }
  return iret;
}
int64_t WriteDataInfo::length() const
{
  return INT_SIZE * 4 + INT64_SIZE * 2;
}

int CloseFileInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&mode_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&crc_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_number_));
  }
  return iret;
}
int CloseFileInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, mode_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, crc_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_number_);
  }
  return iret;
}
int64_t CloseFileInfo::length() const
{
  return INT_SIZE * 3 + INT64_SIZE * 2;
}

int RenameFileInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&new_file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&is_server_));
  }
  return iret;
}

int RenameFileInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, new_file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, is_server_);
  }
  return iret;
}
int64_t RenameFileInfo::length() const
{
  return INT_SIZE * 2 + INT64_SIZE * 2;
}
int ServerMetaInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &capacity_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &available_);
  }
  return iret;
}
int ServerMetaInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, capacity_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, available_);
  }
  return iret;
}

int64_t ServerMetaInfo::length() const
{
  return INT_SIZE * 2;
}
int SegmentHead::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, &size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_bytes(data, data_len, pos, reserve_,SEGMENT_HEAD_RESERVE_SIZE);
  }
  return iret;

}
int SegmentHead::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_bytes(data, data_len, pos, reserve_, SEGMENT_HEAD_RESERVE_SIZE);
  }
  return iret;
}
int64_t SegmentHead::length() const
{
  return INT_SIZE + INT64_SIZE;
}
int SegmentInfo::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, &offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &crc_);
  }
  return iret;
}
int SegmentInfo::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, block_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, file_id_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, crc_);
  }
  return iret;
}
int64_t SegmentInfo::length() const
{
  return INT_SIZE * 3 + INT64_SIZE * 2;
}

int ClientCmdInformation::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, cmd_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, value1_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, value3_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, value4_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, value2_);
  }
  return iret;
}

int ClientCmdInformation::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&cmd_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&value1_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&value3_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&value4_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&value2_));
  }
  return iret;
}

int64_t ClientCmdInformation::length() const
{
  return INT_SIZE * 3 + INT64_SIZE * 2;
}

int MMapOption::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, max_mmap_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, first_mmap_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, per_mmap_size_);
  }
  return iret;
}

int MMapOption::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &max_mmap_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &first_mmap_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &per_mmap_size_);
  }
  return iret;
}

int64_t MMapOption::length() const
{
  return INT_SIZE * 3;
}

int SuperBlock::serialize(char* data, const int64_t data_len, int64_t& pos) const
{
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, MAX_DEV_TAG_LEN);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_bytes(data, data_len, pos, mount_tag_, MAX_DEV_TAG_LEN);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, time_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, MAX_DEV_NAME_LEN);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_bytes(data, data_len, pos, mount_point_, MAX_DEV_NAME_LEN);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int64(data, data_len, pos, mount_point_use_space_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, base_fs_type_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, superblock_reserve_offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, bitmap_start_offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, avg_segment_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, static_cast<int32_t>(block_type_ratio_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, main_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, main_block_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, extend_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, extend_block_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, used_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, used_extend_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, static_cast<int32_t>(hash_slot_ratio_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, hash_slot_size_);
  }
  if (SUCCESS == iret)
  {
    iret = mmap_option_.serialize(data, data_len, pos);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::set_int32(data, data_len, pos, version_);
  }
  return iret;
}

int SuperBlock::deserialize(const char* data, const int64_t data_len, int64_t& pos)
{
  printf("%s", dynamic_parameter_str[0]);
  int32_t iret = NULL != data && data_len - pos >= length() ? SUCCESS : ERROR;
  int32_t str_length = 0;
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &str_length);
  }
  if (SUCCESS == iret)
  {
    iret = str_length > 0 && str_length <= MAX_DEV_NAME_LEN ? SUCCESS : ERROR;
    if (SUCCESS == iret)
    {
      iret = Serialization::get_bytes(data, data_len, pos, mount_tag_, str_length);
    }
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &time_);
  }
  if (SUCCESS == iret)
  {
    str_length = 0;
    iret = Serialization::get_int32(data, data_len, pos, &str_length);
  }

  if (SUCCESS == iret)
  {
    iret = str_length > 0 && str_length <= MAX_DEV_NAME_LEN ? SUCCESS : ERROR;
    if (SUCCESS == iret)
    {
      iret = Serialization::get_bytes(data, data_len, pos, mount_point_, str_length);
    }
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int64(data, data_len, pos, &mount_point_use_space_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&base_fs_type_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &superblock_reserve_offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &bitmap_start_offset_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &avg_segment_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_type_ratio_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &main_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &main_block_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &extend_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &extend_block_size_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &used_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &used_extend_block_count_);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&hash_slot_ratio_));
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &hash_slot_size_);
  }
  if (SUCCESS == iret)
  {
    iret = mmap_option_.deserialize(data, data_len, pos);
  }
  if (SUCCESS == iret)
  {
    iret = Serialization::get_int32(data, data_len, pos, &version_);
  }
  return iret;
}

int64_t SuperBlock::length() const
{
  return 17 * INT_SIZE + INT64_SIZE + MAX_DEV_TAG_LEN + MAX_DEV_NAME_LEN + mmap_option_.length();
}

void SuperBlock::display() const
{
  std::cout << "tag " << mount_tag_ << std::endl;
  std::cout << "mount time " << time_ << std::endl;
  std::cout << "mount desc " << mount_point_ << std::endl;
  std::cout << "max use space " << mount_point_use_space_ << std::endl;
  std::cout << "base filesystem " << base_fs_type_ << std::endl;
  std::cout << "superblock reserve " << superblock_reserve_offset_ << std::endl;
  std::cout << "bitmap start offset " << bitmap_start_offset_ << std::endl;
  std::cout << "avg inner file size " << avg_segment_size_ << std::endl;
  std::cout << "block type ratio " << block_type_ratio_ << std::endl;
  std::cout << "main block count " << main_block_count_ << std::endl;
  std::cout << "main block size " << main_block_size_ << std::endl;
  std::cout << "extend block count " << extend_block_count_ << std::endl;
  std::cout << "extend block size " << extend_block_size_ << std::endl;
  std::cout << "used main block count " << used_block_count_ << std::endl;
  std::cout << "used extend block count " << used_extend_block_count_ << std::endl;
  std::cout << "hash slot ratio " << hash_slot_ratio_ << std::endl;
  std::cout << "hash slot size " << hash_slot_size_ << std::endl;
  std::cout << "first mmap size " << mmap_option_.first_mmap_size_ << std::endl;
  std::cout << "mmap size step " << mmap_option_.per_mmap_size_ << std::endl;
  std::cout << "max mmap size " << mmap_option_.max_mmap_size_ << std::endl;
  std::cout << "version " << version_ << std::endl;
}

const char* dynamic_parameter_str[31] = {
  "log_level",
  "plan_run_flag",
  "task_expired_time",
  "safe_mode_time",
  "max_write_timeout",
  "max_write_file_count",
  "add_primary_block_count",
  "cleanup_write_timeout_threshold",
  "max_use_capacity_ratio",
  "heart_interval",
  "replicate_ratio",
  "replicate_wait_time",
  "compact_delete_ratio",
  "compact_max_load",
  "compact_time_lower",
  "compact_time_upper",
  "max_task_in_machine_nums",
  "discard_newblk_safe_mode_time",
  "discard_max_count",
  "cluster_index",
  "object_dead_max_time",
  "group_count",
  "group_seq",
  "object_clear_max_time",
  "report_block_queue_size",
  "report_block_time_lower",
  "report_block_time_upper",
  "report_block_time_interval",
  "report_block_expired_time",
  "choose_target_server_random_max_nums",
  "max_keepalive_queue_size"
};

} //namespace base
} //namespace neptune
