#include "index_handle.h"
#include "dataserver_define.h"
#include "base/common/ErrorMsg.h"
#include "dfs/util/dfs.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;

IndexHandle::IndexHandle(const std::string& base_path, const uint32_t main_block_id)
{
  //create file_op handle
  std::stringstream tmp_stream;
  tmp_stream << base_path << INDEX_DIR_PREFIX << main_block_id;
  std::string index_path;
  tmp_stream >> index_path;
  file_op_ = new MMapFileOperation(index_path, O_RDWR | O_LARGEFILE | O_CREAT);
  is_load_ = false;
}

IndexHandle::~IndexHandle()
{
  if (file_op_)
  {
    delete file_op_;
    file_op_ = NULL;
  }
}

// create index file. inner format:
// ------------------------------------------------------------------------------------------
// | index header|   hash bucket: each slot hold     |           file meta info             |
// |             |   offset of file's MetaInfo       |                                      |
// ------------------------------------------------------------------------------------------
// | IndexHeader | int32_t | int32_t | ... | int32_t | MetaInfo | MetaInfo | ... | MetaInfo |
// ------------------------------------------------------------------------------------------
int IndexHandle::create(const uint32_t logic_block_id, const int32_t cfg_bucket_size, const MMapOption map_option,
    const DirtyFlag dirty_flag)
{
  //LOG(
  //    INFO,
  //    "index create block: %u index. bucket size: %d, max mmap size: %d, first mmap size: %d, per mmap size: %d, dirty flag: %d",
  //    logic_block_id, cfg_bucket_size, map_option.max_mmap_size_, map_option.first_mmap_size_,
  //    map_option.per_mmap_size_, dirty_flag);
  if (is_load_)
  {
    return EXIT_INDEX_ALREADY_LOADED_ERROR;
  }
  int ret = SUCCESS;
  int64_t file_size = file_op_->get_file_size();
  // file size corrupt
  if (file_size < 0)
  {
    return ERROR;
  }
  else if (file_size == 0) // empty file
  {
    IndexHeader i_header;
    i_header.block_info_.block_id_ = logic_block_id;
    i_header.block_info_.seq_no_ = 1;
    i_header.index_file_size_ = sizeof(IndexHeader) + cfg_bucket_size * sizeof(int32_t);
    i_header.bucket_size_ = cfg_bucket_size;
    i_header.flag_ = dirty_flag;

    // index header + total buckets
    char* init_data = new char[i_header.index_file_size_];
    memcpy(init_data, &i_header, sizeof(IndexHeader));
    memset(init_data + sizeof(IndexHeader), 0, i_header.index_file_size_ - sizeof(IndexHeader));

    // write index header and buckets into to blockfile
    ret = file_op_->pwrite_file(init_data, i_header.index_file_size_, 0);
    delete[] init_data;
    init_data = NULL;
    if (SUCCESS != ret)
      return ret;

    // write to disk as immediately as possible
    ret = file_op_->flush_file();
    if (SUCCESS != ret)
      return ret;
  }
  else //file size > 0, index already exist
  {
    return EXIT_INDEX_UNEXPECT_EXIST_ERROR;
  }

  ret = file_op_->mmap_file(map_option);
  if (SUCCESS != ret) //mmap fail
    return ret;

  is_load_ = true;
  //LOG(
  //    INFO,
  //    "init blockid: %u index successful. data file size: %d, index file size: %d, bucket size: %d, free head offset: %d, seqno: %d, size: %d, filecount: %d, del_size: %d, del_file_count: %d version: %d",
  //    logic_block_id, index_header()->data_file_offset_, index_header()->index_file_size_,
  //    index_header()->bucket_size_, index_header()->free_head_offset_, block_info()->seq_no_, block_info()->size_,
  //    block_info()->file_count_, block_info()->del_size_, block_info()->del_file_count_, block_info()->version_);
  return SUCCESS;
}

int IndexHandle::load(const uint32_t logic_block_id, const int32_t cfg_bucket_size, const MMapOption map_option)
{
  if (is_load_)
  {
    return EXIT_INDEX_ALREADY_LOADED_ERROR;
  }

  int ret = SUCCESS;
  int file_size = file_op_->get_file_size();
  if (file_size < 0)
  {
    return file_size;
  }
  else if (file_size == 0) // empty file
  {
    return EXIT_INDEX_CORRUPT_ERROR;
  }

  //resize mmap size
  MMapOption tmp_map_option = map_option;
  if (file_size > tmp_map_option.first_mmap_size_ && file_size <= tmp_map_option.max_mmap_size_)
  {
    tmp_map_option.first_mmap_size_ = file_size;
  }

  // map file into memory
  ret = file_op_->mmap_file(tmp_map_option);
  if (SUCCESS != ret)
    return ret;

  // check stored logic block id and bucket size
  // meta info corrupt, may be destroyed when created by unexpect interrupt
  if (0 == bucket_size() || 0 == block_info()->block_id_)
  {
    //LOG(ERROR, "Index corrupt error. blockid: %u, bucket size: %d", block_info()->block_id_,
    //    bucket_size());
    return EXIT_INDEX_CORRUPT_ERROR;
  }

  //check file size
  int32_t index_file_size = sizeof(IndexHeader) + bucket_size() * sizeof(int32_t);
  // uncomplete index file
  if (file_size < index_file_size)
  {
    //LOG(ERROR, "Index corrupt error. blockid: %u, bucket size: %d, file size: %d, index file size: %d",
    //    block_info()->block_id_, bucket_size(), file_size, index_file_size);
    return EXIT_INDEX_CORRUPT_ERROR;
  }

  // check bucket_size
  if (cfg_bucket_size != bucket_size())
  {
    //LOG(ERROR, "Index configure error. old bucket size: %d, new bucket size: %d", bucket_size(),
    //    cfg_bucket_size);
    return EXIT_BUCKET_CONFIGURE_ERROR;
  }

  // check block_id
  if (logic_block_id != block_info()->block_id_)
  {
    //LOG(ERROR, "block id conflict. blockid: %u, index blockid: %u", logic_block_id, block_info()->block_id_);
    return EXIT_BLOCKID_CONFLICT_ERROR;
  }

  // check flag, if normal state(C_DATA_CLEAN), do nothing
  if (C_DATA_COMPACT == index_header()->flag_)
  {
    //unfinish compact block
    //LOG(ERROR, "It is a unfinish compact block. blockid: %u", logic_block_id);
    return EXIT_COMPACT_BLOCK_ERROR;
  } else if (C_DATA_HALF == index_header()->flag_)
  {
    // unfinish repl block, coding block
    //LOG(ERROR, "It is a half state block. blockid: %u", logic_block_id);
    return EXIT_HALF_BLOCK_ERROR;
  }

  is_load_ = true;
  //LOG(
  //    INFO,
  //    "load blockid: %u index successful. data file offset: %d, index file size: %d, bucket size: %d, free head offset: %d, seqno: %d, size: %d, filecount: %d, del size: %d, del file count: %d version: %d",
  //    logic_block_id, index_header()->data_file_offset_, index_header()->index_file_size_, bucket_size(),
  //    index_header()->free_head_offset_, block_info()->seq_no_, block_info()->size_, block_info()->file_count_,
  //    block_info()->del_size_, block_info()->del_file_count_, block_info()->version_);
  return SUCCESS;
}

// remove index: unmmap and unlink file
int IndexHandle::remove(const uint32_t logic_block_id)
{
  if (is_load_)
  {
    if (logic_block_id != block_info()->block_id_)
    {
    //  LOG(ERROR, "block id conflict. blockid: %d, index blockid: %d", logic_block_id, block_info()->block_id_);
      return EXIT_BLOCKID_CONFLICT_ERROR;
    }
  }

  int ret = file_op_->munmap_file();
  if (SUCCESS != ret)
    return ret;

  ret = file_op_->unlink_file();
  return ret;
}

// rename index file
int IndexHandle::rename(const uint32_t logic_block_id)
{
  if (is_load_)
  {
    if (logic_block_id != block_info()->block_id_)
    {
    //  LOG(ERROR, "block id conflict. blockid: %d, index blockid: %d", logic_block_id, block_info()->block_id_);
      return EXIT_BLOCKID_CONFLICT_ERROR;
    }
  }

  int ret = file_op_->rename_file();
  return ret;
}

int IndexHandle::flush()
{
  int ret = file_op_->flush_file();
  if (SUCCESS != ret)
  {
  //  LOG(ERROR, "index flush fail. ret: %d, error desc: %s", ret, strerror(errno));
  }
  return ret;
}

int IndexHandle::set_block_dirty_type(const DirtyFlag dirty_flag)
{
  index_header()->flag_ = dirty_flag;
  return flush();
}

int IndexHandle::find_avail_key(uint64_t& key)
{
  // for write, get next sequence number
  if (0 == key)
  {
    key = block_info()->seq_no_;
  } // continue test

  int32_t offset = 0, slot = 0;
  int ret = SUCCESS;
  MetaInfo meta_info;
  int retry_times = MAX_RETRY_TIMES;
  bool found = false;
  do
  {
    // use low 32bit
    slot = static_cast<uint32_t> (key) % bucket_size();
    // the first metainfo node
    offset = bucket_slot()[slot];
    // if this position is empty, use this key
    if (0 == offset)
    {
      found = true;
      break;
    }

    // hash corrupt, find in the list
    for (; offset != 0;)
    {
      ret = file_op_->pread_file(reinterpret_cast<char*> (&meta_info), META_INFO_SIZE, offset);
      if (SUCCESS != ret)
        return ret;
      // compare the low 32bit. if conflict
      if (static_cast<uint32_t> (key) == static_cast<uint32_t> (meta_info.get_key()))
      {
        // if exists, test key + 1
        ++key;
        break;
      }
      offset = meta_info.get_next_meta_offset();
    }

    //this key is not exist in the list
    if (0 == offset)
    {
      found = true;
      break;
    }
  }
  while (retry_times--);

  // assign low 32bit to 64bit
  block_info()->seq_no_ = key + 1;

  if (!found)
  {
  //  LOG(ERROR, "blockid: %u, find avail key fail. new key: %" PRI64_PREFIX "u", block_info()->block_id_, key);
    return EXIT_CREATE_FILEID_ERROR;
  }

  //LOG(DEBUG, "blockid: %u, get key: %" PRI64_PREFIX "u, seqno: %u", block_info()->block_id_, key,
  //    block_info()->seq_no_);
  return SUCCESS;
}

void IndexHandle::reset_avail_key(uint64_t key)
{
  if (block_info()->seq_no_ <= key)
  {
    block_info()->seq_no_ = key + 1;
    // overlap ...
  }
}

int IndexHandle::check_block_version(int32_t& remote_version)
{
  //LOG(DEBUG, "block version. blockid: %u, remote version: %u, local version: %u", block_info()->block_id_,
  //    remote_version, block_info()->version_);
  if (remote_version != block_info()->version_)
  {
    //we assume that the difference between the version number greater than BLOCK_VERSION_MAGIC_NUM is illegal
    if (remote_version > block_info()->version_ + BLOCK_VERSION_MAGIC_NUM)
    {
    //  LOG(ERROR, "block version error. blockid: %u, remote version: %d, local version: %d",
    //      block_info()->block_id_, remote_version, block_info()->version_);
      return EXIT_BLOCK_DS_VERSION_ERROR;
    }
    else if (remote_version < block_info()->version_ - BLOCK_VERSION_MAGIC_NUM)
    {
      //LOG(ERROR, "block version error. blockid: %u, remote version: %d, local version: %d",
      //    block_info()->block_id_, remote_version, block_info()->version_);
      return EXIT_BLOCK_NS_VERSION_ERROR;
    }

    //if local version less than ns version, and the difference is little than 2, replace the local version with ns version
    if (remote_version > block_info()->version_ && remote_version <= (block_info()->version_
        + BLOCK_VERSION_MAGIC_NUM))
    {
      //LOG(ERROR,
      //    "remote version is larger, set block version. blockid: %u, remote version: %u, local version: %u",
      //    block_info()->block_id_, remote_version, block_info()->version_);
      block_info()->version_ = remote_version;
    }
    else
    {
      //LOG(ERROR,
      //    "block version is larger, set remote version. blockid: %u, remote version: %u, local version: %u",
      //    block_info()->block_id_, remote_version, block_info()->version_);
      remote_version = block_info()->version_;
    }
  }

  return SUCCESS;
}

int IndexHandle::reset_block_version()
{
  block_info()->version_ = 1;
  return SUCCESS;
}

//write at the end of the list
int IndexHandle::write_segment_meta(const uint64_t key, const RawMeta& meta)
{
  int32_t current_offset = 0, previous_offset = 0;
  int ret = hash_find(key, current_offset, previous_offset);
  if (SUCCESS == ret) // check not exists
  {
    return EXIT_META_UNEXPECT_FOUND_ERROR;
  }
  else if (EXIT_META_NOT_FOUND_ERROR != ret)
  {
    return ret;
  }

  int32_t slot = static_cast<uint32_t> (key) % bucket_size();
  return hash_insert(slot, previous_offset, meta);
}

int IndexHandle::read_segment_meta(const uint64_t key, RawMeta& meta)
{
  int32_t current_offset = 0, previous_offset = 0;
  // find
  int ret = hash_find(key, current_offset, previous_offset);
  if (SUCCESS == ret) //exist
  {
    ret = file_op_->pread_file(reinterpret_cast<char*> (&meta), RAW_META_SIZE, current_offset);
    if (SUCCESS != ret)
      return ret;
  }
  else
  {
    return ret;
  }

  return SUCCESS;
}

int IndexHandle::override_segment_meta(const uint64_t key, const RawMeta& meta)
{
  // find
  int32_t current_offset = 0, previous_offset = 0;
  int ret = hash_find(key, current_offset, previous_offset);
  //exist, update
  if (SUCCESS == ret)
  {
    //get next meta offset
    MetaInfo tmp_meta;
    ret = file_op_->pread_file(reinterpret_cast<char*> (&tmp_meta), META_INFO_SIZE, current_offset);
    if (SUCCESS != ret)
      return ret;

    tmp_meta.set_raw_meta(meta);
    ret = file_op_->pwrite_file(reinterpret_cast<const char*> (&tmp_meta), META_INFO_SIZE, current_offset);
    if (SUCCESS != ret)
      return ret;
  }
  else if (EXIT_META_NOT_FOUND_ERROR != ret)
  {
    return ret;
  }
  else if (EXIT_META_NOT_FOUND_ERROR == ret) // nonexists, insert
  {
    // insert
    int32_t slot = static_cast<uint32_t> (key) % bucket_size();
    ret = hash_insert(slot, previous_offset, meta);
    if (SUCCESS != ret)
      return ret;
  }
  return ret;
}

int IndexHandle::batch_override_segment_meta(const RawMetaVec& meta_list)
{
  int ret = SUCCESS;
  for (RawMetaVecConstIter mit = meta_list.begin(); mit != meta_list.end(); ++mit)
  {
    ret = override_segment_meta(mit->get_key(), *mit);
    if (SUCCESS != ret)
    {
      return ret;
    }
  }
  return SUCCESS;
}

int IndexHandle::update_segment_meta(const uint64_t key, const RawMeta& meta)
{
  // find
  int32_t current_offset = 0, previous_offset = 0;
  int ret = hash_find(key, current_offset, previous_offset);
  if (SUCCESS == ret) // exist
  {
    MetaInfo tmp_meta;
    ret = file_op_->pread_file(reinterpret_cast<char*> (&tmp_meta), META_INFO_SIZE, current_offset);
    if (SUCCESS != ret)
      return ret;

    tmp_meta.set_raw_meta(meta);
    ret = file_op_->pwrite_file(reinterpret_cast<const char*> (&tmp_meta), META_INFO_SIZE, current_offset);
    if (SUCCESS != ret)
      return ret;
  }
  else
  {
    return ret;
  }

  return SUCCESS;
}

int IndexHandle::delete_segment_meta(const uint64_t key)
{
  // find
  int32_t current_offset = 0, previous_offset = 0;
  int ret = hash_find(key, current_offset, previous_offset);
  if (SUCCESS != ret)
    return ret;

  MetaInfo meta_info;
  ret = file_op_->pread_file(reinterpret_cast<char*> (&meta_info), META_INFO_SIZE, current_offset);
  if (SUCCESS != ret)
    return ret;

  int32_t tmp_pos = meta_info.get_next_meta_offset();

  int32_t slot = static_cast<uint32_t> (key) % bucket_size();
  // the header of the list
  if (0 == previous_offset)
  {
    bucket_slot()[slot] = tmp_pos;
  }
  else // delete from list, modify previous
  {
    MetaInfo pre_meta_info;
    ret = file_op_->pread_file(reinterpret_cast<char*> (&pre_meta_info), META_INFO_SIZE, previous_offset);
    if (SUCCESS != ret)
      return ret;

    pre_meta_info.set_next_meta_offset(tmp_pos);
    ret = file_op_->pwrite_file(reinterpret_cast<const char*> (&pre_meta_info), META_INFO_SIZE, previous_offset);
    if (SUCCESS != ret)
      return ret;
  }

  // get free head list, be head.
  meta_info.set_next_meta_offset(index_header()->free_head_offset_);
  ret = file_op_->pwrite_file(reinterpret_cast<const char*> (&meta_info), META_INFO_SIZE, current_offset);
  if (SUCCESS != ret)
    return ret;
  // add to free head list, if bread down at this time, current offset will not be used for ever
  index_header()->free_head_offset_ = current_offset;

  return SUCCESS;
}

int IndexHandle::traverse_segment_meta(RawMetaVec& raw_metas)
{
  int ret = SUCCESS;
  // traverse hash bucket
  for (int32_t slot = 0; slot < bucket_size(); ++slot)
  {
    // to each hash bucket slot, traverse list
    for (int32_t current_offset = bucket_slot()[slot]; current_offset != 0;)
    {
      if (current_offset >= index_header()->index_file_size_)
      {
        return EXIT_META_OFFSET_ERROR;
      }

      MetaInfo tmp_meta;
      ret = file_op_->pread_file(reinterpret_cast<char*> (&tmp_meta), META_INFO_SIZE, current_offset);
      if (SUCCESS != ret)
        return ret;
      raw_metas.push_back(tmp_meta.get_raw_meta());

      current_offset = tmp_meta.get_next_meta_offset();
    }
  }
  return SUCCESS;
}

int IndexHandle::traverse_sorted_segment_meta(RawMetaVec& meta)
{
  int ret = traverse_segment_meta(meta);
  if (SUCCESS != ret)
    return ret;

  std::sort(meta.begin(), meta.end(), RawMetaSort());
  return SUCCESS;
}

int IndexHandle::update_block_info(const OperType oper_type, const uint32_t modify_size)
{
  if (0 == block_info()->block_id_)
  {
    return EXIT_BLOCKID_ZERO_ERROR;
  }

  // to each operate type, update statistics eg, version count size stuff etc
  if (C_OPER_INSERT == oper_type)
  {
    ++block_info()->version_;
    ++block_info()->file_count_;
    block_info()->size_ += modify_size;
  }
  else if (C_OPER_DELETE == oper_type)
  {
    ++block_info()->del_file_count_;
    block_info()->del_size_ += modify_size;
  }
  else if (C_OPER_UNDELETE == oper_type)
  {
    --block_info()->del_file_count_;
    block_info()->del_size_ -= modify_size;
  }
  else if (C_OPER_UPDATE == oper_type)
  {
    ++block_info()->version_;
    block_info()->size_ += modify_size;
  }

  LOG(
      DEBUG,
      "update block info. blockid: %u, version: %u, file count: %u, size: %u, del file count: %u, del size: %u, seq no: %u, oper type: %d",
      block_info()->block_id_, block_info()->version_, block_info()->file_count_, block_info()->size_,
      block_info()->del_file_count_, block_info()->del_size_, block_info()->seq_no_, oper_type);
  return SUCCESS;
}

int IndexHandle::copy_block_info(const BlockInfo* blk_info)
{
  if (NULL == blk_info)
  {
    return EXIT_POINTER_NULL;
  }

  memcpy(block_info(), blk_info, sizeof(BlockInfo));
  return SUCCESS;
}

// find key in the block
int IndexHandle::hash_find(const uint64_t key, int32_t& current_offset, int32_t& previous_offset)
{
  // find bucket slot
  int32_t slot = static_cast<uint32_t> (key) % bucket_size();
  previous_offset = 0;
  MetaInfo meta_info;
  int ret = SUCCESS;
  // find in the list
  for (int32_t pos = bucket_slot()[slot]; pos != 0;)
  {
    ret = file_op_->pread_file(reinterpret_cast<char*> (&meta_info), META_INFO_SIZE, pos);
    if (SUCCESS != ret)
      return ret;

    if (hash_compare(key, meta_info.get_key()))
    {
      current_offset = pos;
      return SUCCESS;
    }

    previous_offset = pos;
    pos = meta_info.get_next_meta_offset();
  }
  return EXIT_META_NOT_FOUND_ERROR;
}

// insert meta into the tail(the current tail is previous_offset) of bucket(slot)
int IndexHandle::hash_insert(const int32_t slot, const int32_t previous_offset, const RawMeta& meta)
{
  int ret = SUCCESS;
  MetaInfo tmp_meta_info;
  int32_t current_offset = 0;
  // get insert offset
  // reuse the node in the free list
  if (0 != index_header()->free_head_offset_)
  {
    ret = file_op_->pread_file(reinterpret_cast<char*> (&tmp_meta_info), META_INFO_SIZE,
        index_header()->free_head_offset_);
    if (SUCCESS != ret)
      return ret;

    current_offset = index_header()->free_head_offset_;
    index_header()->free_head_offset_ = tmp_meta_info.get_next_meta_offset();
  }
  else // expand index file
  {
    current_offset = index_header()->index_file_size_;
    index_header()->index_file_size_ += META_INFO_SIZE;
  }

  MetaInfo meta_info(meta);
  ret = file_op_->pwrite_file(reinterpret_cast<const char*> (&meta_info), META_INFO_SIZE, current_offset);
  if (SUCCESS != ret)
    return ret;

  // previous_offset the last elem in the list, modify node
  if (0 != previous_offset)
  {
    ret = file_op_->pread_file(reinterpret_cast<char*> (&tmp_meta_info), META_INFO_SIZE, previous_offset);
    if (SUCCESS != ret)
      return ret;

    tmp_meta_info.set_next_meta_offset(current_offset);
    ret = file_op_->pwrite_file(reinterpret_cast<const char*> (&tmp_meta_info), META_INFO_SIZE, previous_offset);
    if (SUCCESS != ret)
      return ret;
  }
  else //the first elem in bucket slot, set slot
  {
    bucket_slot()[slot] = current_offset;
  }
  return SUCCESS;
}

}
}
}
