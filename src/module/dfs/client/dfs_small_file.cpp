#include "dfs_small_file.h"

using namespace neptune::dfs;

DfsSmallFile::DfsSmallFile()
{
}

DfsSmallFile::~DfsSmallFile()
{
}

int DfsSmallFile::open(const char *file_name, const char *suffix, const int flags, ... )
{
  return open_ex(file_name, suffix, flags);
}

int64_t DfsSmallFile::read(void* buf, const int64_t count)
{
  return read_ex(buf, count, offset_);
}

int64_t DfsSmallFile::readv2(void* buf, const int64_t count, DfsFileStat* file_info)
{
  int64_t ret = EXIT_GENERAL_ERROR;
  int64_t offset = offset_;
  if (0 == offset && NULL == file_info)
  {
    //LOG(ERROR, "null DfsFilestat");
  }
  else
  {
    ret = read_ex(buf, count, offset_, true, FILE_PHASE_READ_FILE_V2);

    if (0 == offset && ret >= 0 && meta_seg_->file_info_ != NULL)
    {
      wrap_file_info(file_info, meta_seg_->file_info_);
    }
  }
  return ret;
}

int64_t DfsSmallFile::write(const void* buf, const int64_t count)
{
  return write_ex(buf, count, offset_);
}

int64_t DfsSmallFile::lseek(const int64_t offset, const int whence)
{
  return lseek_ex(offset, whence);
}

int64_t DfsSmallFile::pread(void *buf, const int64_t count, const int64_t offset)
{
  return pread_ex(buf, count, offset);
}

int64_t DfsSmallFile::pwrite(const void *buf, const int64_t count, const int64_t offset)
{
  return pwrite_ex(buf, count, offset);
}

int DfsSmallFile::fstat(DfsFileStat* file_info, const DfsStatType mode)
{
  int ret = ERROR;
  if (file_info != NULL)
  {
    FileInfo inner_file_info;
    ret = fstat_ex(&inner_file_info, mode);
    if (SUCCESS == ret)
    {
      wrap_file_info(file_info, &inner_file_info);
    }
  }
  else
  {
    //LOG(ERROR, "null DfsFilestat");
  }
  return ret;
}

int DfsSmallFile::close()
{
  return close_ex();
}

int64_t DfsSmallFile::get_file_length()
{
  int64_t ret_len = -1;

  if (meta_seg_ != NULL && meta_seg_->file_info_ != NULL)
  {
    ret_len = meta_seg_->file_info_->size_;
  }
  else
  {
    FileInfo file_info;
    int ret = fstat_ex(&file_info, NORMAL_STAT);
    if (SUCCESS == ret)
    {
      ret_len = file_info.size_;
    }
  }
  return ret_len;
}

int DfsSmallFile::unlink(const char* file_name, const char* suffix, int64_t& file_size, const DfsUnlinkType action)
{
  int ret = open_ex(file_name, suffix, T_UNLINK);
  if (SUCCESS != ret)
  {
    //LOG(DEBUG, "dfs unlink fail, file_name: %s, action: %d", (file_name == NULL) ? file_name : "NULL", action);
  }
  else
  {
    meta_seg_->unlink_action_ = action;
    get_meta_segment(0, NULL, 0);
    ret = unlink_process();
    if (SUCCESS == ret && (DELETE == action || UNDELETE == action))
    {
      file_size = meta_seg_->seg_info_.size_;
    }
  }
  return ret;
}

int64_t DfsSmallFile::get_segment_for_read(const int64_t offset, char* buf, const int64_t count)
{
  return get_meta_segment(offset, buf,
                          count > ClientConfig::segment_size_ ? ClientConfig::segment_size_ : count);
}

int64_t DfsSmallFile::get_segment_for_write(const int64_t offset, const char* buf, const int64_t count)
{
  return get_meta_segment(offset, buf,
                          count > ClientConfig::segment_size_ ? ClientConfig::segment_size_ : count);
}

int DfsSmallFile::read_process(int64_t& read_size, const InnerFilePhase read_file_phase)
{
  int ret = read_process_ex(read_size, read_file_phase);
  if (SUCCESS != ret)
  {
    get_block_info(*meta_seg_, flags_);
  }
  meta_seg_->reset_status();
  return ret;
}

int DfsSmallFile::write_process()
{
  int ret = SUCCESS;
  // just retry this block
  processing_seg_list_[0]->status_ = SEG_STATUS_CREATE_OVER;
  // write data
  if ((ret = process(FILE_PHASE_WRITE_DATA)) != SUCCESS)
  {
    //LOG(ERROR, "write data fail, ret: %d", ret);
  }
  return ret;
}

int32_t DfsSmallFile::finish_write_process(const int status)
{
  int32_t count = 0;
  SEG_DATA_LIST_ITER it = processing_seg_list_.begin();

  if (SUCCESS == status)
  {
    count = processing_seg_list_.size();
    for (; it != processing_seg_list_.end(); it++)
    {
      if ((*it)->delete_flag_)
      {
        gDelete(*it);
      }
    }
    processing_seg_list_.clear();
  }
  else
  {
    while (it != processing_seg_list_.end())
    {
      // for small file, segment just write, not close
      if (SEG_STATUS_BEFORE_CLOSE_OVER == (*it)->status_)
      {
        if ((*it)->delete_flag_)
        {
          gDelete(*it);
        }
        it = processing_seg_list_.erase(it);
        count++;
      }
      else
      {
        ++it;
      }
    }
  }

  return count;
}

int DfsSmallFile::close_process()
{
  int ret = ERROR;

  get_meta_segment(0, NULL, 0);

  if ((ret = process(FILE_PHASE_CLOSE_FILE)) != SUCCESS)
  {
    //LOG(ERROR, "close dfs file fail, ret: %d", ret);
  }
  return ret;
}

int DfsSmallFile::unlink_process()
{
  int ret = SUCCESS;
  if ((ret = process(FILE_PHASE_UNLINK_FILE)) != SUCCESS)
  {
    //LOG(ERROR, "unlink file fail, ret: %d", ret);
  }
  return ret;
}

int DfsSmallFile::wrap_file_info(DfsFileStat* file_stat, FileInfo* file_info)
{
  if (file_stat != NULL && file_info != NULL)
  {
    file_stat->file_id_ = file_info->id_;
    file_stat->offset_ = file_info->offset_;
    file_stat->size_ = file_info->size_;
    file_stat->usize_ = file_info->usize_;
    file_stat->modify_time_ = file_info->modify_time_;
    file_stat->create_time_ = file_info->create_time_;
    file_stat->flag_ = file_info->flag_;
    file_stat->crc_ = file_info->crc_;
  }
  return SUCCESS;
}
