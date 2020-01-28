#include "meta_hash_helper.h"
#include "meta_server_define.h"
#include "base/common/Serialization.h"
#include "stream.h"

namespace neptune {
namespace dfs {

int32_t FileName::length(const char* data)
{
  int32_t length = 0;
  if (data != NULL)
  {
    length = *((unsigned char*)data) + 1;
    //length = data[0] & 0x7F;
    //if (length > 0)
    //{
    //  length += (data[0] & 0x80) ? (NAME_EXTRA_SIZE + 1): 1;
    
  }
  return length;
}

const char* FileName::name(const char* data)
{
  const char* ret_name = data;
  if (length(data) > 0)
  {
    ret_name = data + 1;
  }
  return ret_name;
}

FragMeta::FragMeta() :
  file_id_(0), offset_(0), block_id_(0), size_(0)
{
}

FragMeta::FragMeta(uint32_t block_id, uint64_t file_id, int64_t offset, int32_t size) :
  file_id_(file_id), offset_(offset), block_id_(block_id), size_(size)
{
}

int64_t FragMeta::get_length()
{
  return 2 * sizeof(int64_t) + 2 * sizeof(int32_t);
}

bool FragMeta::operator < (const FragMeta& right) const
{
  return offset_ < right.offset_;
}

int FragMeta::serialize(char* data, const int64_t buff_len, int64_t& pos) const
{
  int ret = ERROR;
  if (buff_len - pos >= get_length())
  {
    Serialization::set_int32(data, buff_len, pos, block_id_);
    Serialization::set_int64(data, buff_len, pos, file_id_);
    Serialization::set_int64(data, buff_len, pos, offset_);
    Serialization::set_int32(data, buff_len, pos, size_);
    ret = SUCCESS;
  }
  return ret;
}

int FragMeta::deserialize(char* data, const int64_t data_len, int64_t& pos)
{
  int ret = Serialization::get_int32(data, data_len, pos, reinterpret_cast<int32_t*>(&block_id_));
  if (SUCCESS == ret)
  {
    ret = Serialization::get_int64(data, data_len, pos, reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == ret)
  {
    ret = Serialization::get_int64(data, data_len, pos, &offset_);
  }
  if (SUCCESS == ret)
  {
    ret = Serialization::get_int32(data, data_len, pos, &size_);
  }
  return ret;
}

int FragMeta::serialize(Stream& output) const
{
  int ret = ERROR;
  ret = output.set_int32(block_id_);
  if (SUCCESS == ret)
  {
    ret = output.set_int64(file_id_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(offset_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int32(size_);
  }
  return ret;
}

int FragMeta::deserialize(Stream& input)
{
  int ret = input.get_int32(reinterpret_cast<int32_t*>(&block_id_));
  if (SUCCESS == ret)
  {
    ret = input.get_int64(reinterpret_cast<int64_t*>(&file_id_));
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&offset_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int32(&size_);
  }
  return ret;
}

FragInfo::FragInfo() :
  cluster_id_(0), had_been_split_(false)
{
}

FragInfo::FragInfo(const std::vector<FragMeta>& v_frag_meta) :
  cluster_id_(0), had_been_split_(false),
  v_frag_meta_(v_frag_meta)
{
}

FragInfo::~FragInfo()
{
}

int64_t FragInfo::get_length() const
{
  return sizeof(int32_t) * 2 + v_frag_meta_.size() * FragMeta::get_length();
}

int FragInfo::serialize(char* data, const int64_t buff_len, int64_t& pos) const
{
  int ret = ERROR;
  if (buff_len - pos >= get_length())
  {
    Serialization::set_int32(data, buff_len, pos, cluster_id_);
    int32_t frag_count = static_cast<int32_t>(v_frag_meta_.size());
    if (had_been_split_)
    {
      frag_count |= (1 << 31);
    }
    Serialization::set_int32(data, buff_len, pos, frag_count);
    std::vector<FragMeta>::const_iterator iter = v_frag_meta_.begin();
    for (; iter != v_frag_meta_.end(); iter++)
    {
      (*iter).serialize(data, buff_len, pos);
    }
    ret = SUCCESS;
  }
  return ret;
}

int FragInfo::serialize(Stream& output) const
{
  int ret = ERROR;
  ret = output.set_int32(cluster_id_);
  int32_t frag_count = static_cast<int32_t>(v_frag_meta_.size());
  if (SUCCESS == ret)
  {
    ret = output.set_int32(frag_count);
  }
  std::vector<FragMeta>::const_iterator iter = v_frag_meta_.begin();
  for (; iter != v_frag_meta_.end(); iter++)
  {
    if (SUCCESS == ret)
    {
      ret = (*iter).serialize(output);
    }
  }
  return ret;
}

int FragInfo::deserialize(char* data, const int64_t data_len, int64_t& pos)
{
  int32_t frg_count = 0;
  int ret = Serialization::get_int32(data, data_len, pos, &cluster_id_);
  if (SUCCESS == ret)
  {
    ret = Serialization::get_int32(data, data_len, pos, &frg_count);
    had_been_split_ = (frg_count >> 31);
    frg_count = frg_count & ~(1 << 31);
  }
  if (SUCCESS == ret)
  {
    v_frag_meta_.clear();
    FragMeta tmp;
    for (int i = 0; i < frg_count && SUCCESS == ret; i ++)
    {
      ret = tmp.deserialize(data, data_len, pos);
      v_frag_meta_.push_back(tmp);
    }
  }
  return ret;
}

int FragInfo::deserialize(Stream& input)
{
  int32_t frg_count = 0;
  int ret = input.get_int32(&cluster_id_);
  if (SUCCESS == ret)
  {
    ret = input.get_int32(&frg_count);
  }
  if (SUCCESS == ret)
  {
    v_frag_meta_.clear();
    FragMeta tmp;
    for (int i = 0; i < frg_count && SUCCESS == ret; i ++)
    {
      ret = tmp.deserialize(input);
      v_frag_meta_.push_back(tmp);
    }
  }
  return ret;
}

int64_t FragInfo::get_last_offset() const
{
  int64_t last_offset = 0;
  if (!v_frag_meta_.empty())
  {
    std::vector<FragMeta>::const_iterator it = v_frag_meta_.end() - 1;
    last_offset = it->offset_ + it->size_;
  }
  return last_offset;
}

void FragInfo::dump() const
{
  //LOG(DEBUG, "cluster_id: %d\n", cluster_id_);
  //LOG(DEBUG, "had_been_split: %d\n", had_been_split_);
  //LOG(DEBUG, "frag_size : %zd\n", v_frag_meta_.size());

  std::vector<FragMeta>::const_iterator iter = v_frag_meta_.begin();
  for (; iter != v_frag_meta_.end(); iter++)
  {
    //LOG(DEBUG, "offset: %"PRI64_PREFIX"d  ", (*iter).offset_);
    //LOG(DEBUG, "file_id: %"PRI64_PREFIX"u  ", (*iter).file_id_);
    //LOG(DEBUG, "size: %d  ", (*iter).size_);
    //LOG(DEBUG, "block_id: %u\n", (*iter).block_id_);
  }
}

void FragInfo::push_back(const FragInfo& new_frag_info)
{
  std::vector<FragMeta>::const_iterator iter = new_frag_info.v_frag_meta_.begin();
  for(; iter != new_frag_info.v_frag_meta_.end(); iter++)
  {
    v_frag_meta_.push_back(*iter);
  }
}

FileMetaInfo::FileMetaInfo() :
  pid_(-1), id_(0), create_time_(0), modify_time_(0),
  size_(0), ver_no_(0)
{
}

FileMetaInfo::FileMetaInfo(const FileMetaInfo& file_info)
{
  pid_ = file_info.pid_;
  id_ = file_info.id_;
  create_time_ = file_info.create_time_;
  modify_time_ = file_info.modify_time_;
  size_ = file_info.size_;
  ver_no_ = file_info.ver_no_;
  name_ = file_info.name_;
}

FileMetaInfo::~FileMetaInfo()
{
}

const char* FileMetaInfo::get_real_name() const
{
  return name_.length() > 0 ? name_.data() + 1 : NULL;
}

bool FileMetaInfo::is_file() const
{
  return ((pid_ >> 63) & 0x01);
}

// length to serialize
int64_t FileMetaInfo::length() const
{
  return INT64_SIZE * 3 + INT_SIZE * 2 + INT16_SIZE +
    Serialization::get_string_length(get_real_name());
}

int FileMetaInfo::serialize(Stream& output) const
{
  int ret = output.set_string(get_real_name());
  if (SUCCESS == ret)
  {
    ret = output.set_int64(pid_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(id_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int32(create_time_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int32(modify_time_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int64(size_);
  }
  if (SUCCESS == ret)
  {
    ret = output.set_int16(ver_no_);
  }
  return ret;
}

int FileMetaInfo::deserialize(Stream& input)
{
  int ret = input.get_string(name_);
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&pid_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&id_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int32(&create_time_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int32(&modify_time_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int64(&size_);
  }
  if (SUCCESS == ret)
  {
    ret = input.get_int16(&ver_no_);
  }
  return ret;
}

MetaInfo::MetaInfo()
{
}

MetaInfo::MetaInfo(const MetaInfo& meta_info) :
  file_info_(meta_info.file_info_), frag_info_(meta_info.frag_info_)
{
}

MetaInfo::MetaInfo(FragInfo& frag_info) :
  frag_info_(frag_info)
{
}

void MetaInfo::copy_no_frag(const MetaInfo& meta_info)
{
  file_info_ = meta_info.file_info_;
}

int64_t MetaInfo::length() const
{
  return file_info_.length();
}

int MetaInfo::serialize(Stream& output) const
{
  return file_info_.serialize(output);
}

int MetaInfo::deserialize(Stream& input)
{
  return file_info_.deserialize(input);
}

MsRuntimeGlobalInformation MsRuntimeGlobalInformation::instance_;
MsRuntimeGlobalInformation& MsRuntimeGlobalInformation::instance()
{
  return instance_;
}

AppIdUid::AppIdUid(const int64_t app_id, const int64_t uid)
  :app_id_(app_id), uid_(uid)
{
}
bool AppIdUid::operator < (const AppIdUid& right) const
{
  if (app_id_ < right.app_id_)
  {
    return true;
  }
  if (app_id_ > right.app_id_)
  {
    return false;
  }
  return uid_ < right.uid_;
}

int64_t AppIdUid::get_hash() const
{
  HashHelper helper(app_id_, uid_);
  return CStringUtil::murMurHash((const void*)&helper, sizeof(helper)) % MAX_BUCKET_ITEM_DEFAULT;
}

} //namespace dfs
} //namespace neptune

