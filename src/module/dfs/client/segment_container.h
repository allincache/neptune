#ifndef N_DFS_CLIENT_SEGMENT_CONTAINER_H_
#define N_DFS_CLIENT_SEGMENT_CONTAINER_H_

#include "base/common/Memory.h"
#include "base/common/Internal.h"
#include "base/common/ErrorMsg.h"
#include "base/fs/FileOp.h"

namespace neptune {
namespace dfs {

template< typename V >
class SegmentContainer
{
 public:
  typedef V SEG_INFO_LIST;
  typedef typename V::iterator SEG_INFO_ITER;
  typedef typename V::const_iterator SEG_INFO_CONST_ITER;

  SegmentContainer();
  virtual ~SegmentContainer();

  int load_file(const char* name);
  int dump(char* buf, const int32_t size) const;
  int remove();
  int32_t get_data_size() const;    // get raw data size of segment head and data
  int64_t get_file_size() const;    // get size that segments contain
  int32_t get_segment_size() const; // get segment count
  V& get_seg_info();

  virtual int load() = 0;
  virtual int add_segment(const SegmentInfo& seg_info) = 0;
  virtual int validate(const int64_t total_size = 0) = 0;
  virtual int save() = 0;

 protected:
  void clear();
  void clear_info();
  int init_file_op(const char* name, const int mode);
  int lock_file(const char* name);
  int unlock_file(const char* name);

 protected:
  SegmentHead seg_head_;
  FileOperation* file_op_;
  V seg_info_;
};

template< typename V >
SegmentContainer<V>::SegmentContainer() : file_op_(NULL)
{
}

template< typename V >
SegmentContainer<V> ::~SegmentContainer()
{
  if (file_op_ != NULL)
  {
    unlock_file(file_op_->get_file_name());
    gDelete(file_op_);
  }
}

template< typename V >
int SegmentContainer<V>::load_file(const char* name)
{
  int ret = init_file_op(name, O_RDWR);
  if (SUCCESS == ret)
  {
    ret = load();
  }
  return ret;
}

template< typename V >
int32_t SegmentContainer<V>::get_data_size() const
{
  return sizeof(SegmentHead) + seg_head_.count_ * sizeof(SegmentInfo);
}

template< typename V >
int64_t SegmentContainer<V>::get_file_size() const
{
  return seg_head_.size_;
}

template< typename V >
int32_t SegmentContainer<V>::get_segment_size() const
{
  return seg_head_.count_;
}

template< typename V >
V& SegmentContainer<V>::get_seg_info()
{
  return seg_info_;
}


template< typename V >
int SegmentContainer<V>::dump(char* buf, const int32_t size) const
{
  int ret = ERROR;
  if (size >= get_data_size())
  {
    memcpy(buf, &seg_head_, sizeof(SegmentHead));
    char* pos = buf + sizeof(SegmentHead);
    SEG_INFO_CONST_ITER it;
    for (it = seg_info_.begin(); it != seg_info_.end(); ++it)
    {
      memcpy(pos, &(*it), sizeof(SegmentInfo));
      pos += sizeof(SegmentInfo);
    }
    ret = SUCCESS;
  }
  return ret;
}

template< typename V >
int SegmentContainer<V>::remove()
{
  int ret = SUCCESS;
  if (NULL == file_op_)
  {
    LOG(ERROR, "remove file fail, not initialized");
    ret = ERROR;
  }
  else if ((ret = file_op_->unlink_file()) != SUCCESS)
  {
    LOG(ERROR, "remove file fail, ret: %d", ret);
  }
  else
  {
    unlock_file(file_op_->get_file_name());
    LOG(INFO, "remove file success");
  }
  return ret;
}

template< typename V >
void SegmentContainer<V>::clear()
{
  memset(&seg_head_, 0, sizeof(SegmentHead));
  clear_info();
}

template< typename V >
void SegmentContainer<V>::clear_info()
{
  seg_info_.clear();
}

template< typename V >
int SegmentContainer<V>::init_file_op(const char* name, const int mode)
{
  int ret = ERROR;
  if (name != NULL)
  {
    if (file_op_ != NULL)
    {
      unlock_file(file_op_->get_file_name());
      gDelete(file_op_);
    }

    file_op_ = new FileOperation(name, mode);
    if ((ret = file_op_->open_file()) < 0)
    {
      LOG(WARN, "open file fail: %s, %s", name, strerror(errno));
    }
    else if (lock_file(file_op_->get_file_name()) != SUCCESS)
    {
      LOG(WARN, "file is busy:  %s", file_op_->get_file_name());
      ret = EXIT_FILE_BUSY_ERROR;
    }
    else
    {
      ret = SUCCESS;
    }
  }
  return ret;
}

template< typename V >
int SegmentContainer<V>::lock_file(const char* name)
{
  int ret = ERROR;
  if (NULL != name)
  {
    int fd = ::open(name, O_RDWR);
    if (fd > 0)
    {
      struct flock f_lock;
      f_lock.l_type = F_WRLCK;
      f_lock.l_whence = SEEK_SET;
      f_lock.l_start = 0;
      f_lock.l_len = 0;
      ret = fcntl(fd, F_GETLK, &f_lock);

      // consider checking lock fail as not own a lock
      if (ret != 0 || f_lock.l_type == F_UNLCK)
      {
        f_lock.l_type = F_WRLCK;
        ret = fcntl(fd, F_SETLK, &f_lock) == 0 ? SUCCESS : ERROR;
      }
      else
      {
        ret = ERROR;             // lock is already occupid, can't get lock
      }
      ::close(fd);
    }
  }
  return ret;
}

template< typename V >
int SegmentContainer<V>::unlock_file(const char* name)
{
  int ret = ERROR;
  if (NULL != name)
  {
    int fd = ::open(name, O_RDWR);
    if (fd > 0)
    {
      struct flock f_lock;
      f_lock.l_type = F_UNLCK;
      f_lock.l_whence = SEEK_SET;
      f_lock.l_start = 0;
      f_lock.l_len = 0;

      ret = fcntl(fd, F_SETLK, &f_lock) == 0 ? SUCCESS : ERROR;
      ::close(fd);
    }
  }
  return ret;
}


}
}
#endif
