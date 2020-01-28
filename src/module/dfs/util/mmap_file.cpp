#include "dfs/util/dfs.h"
#include "mmap_file.h"

namespace neptune {
namespace dfs {

MMapFile::MMapFile() :
  size_(0), fd_(-1), data_(NULL)
{
}

MMapFile::MMapFile(const int fd) :
  size_(0), fd_(fd), data_(NULL)
{
}

MMapFile::MMapFile(const MMapOption& mmap_option, const int fd) :
  size_(0), fd_(fd), data_(NULL)
{
  mmap_file_option_.max_mmap_size_ = mmap_option.max_mmap_size_;
  mmap_file_option_.per_mmap_size_ = mmap_option.per_mmap_size_;
  mmap_file_option_.first_mmap_size_ = mmap_option.first_mmap_size_;
}

MMapFile::~MMapFile()
{
  if (data_)
  {
    LOG(INFO, "mmap file destruct, fd: %d maped size: %d, data: %p", fd_, size_, data_);
    msync(data_, size_, MS_SYNC); // make sure synced
    munmap(data_, size_);

    mmap_file_option_.max_mmap_size_ = 0;
    mmap_file_option_.per_mmap_size_ = 0;
    mmap_file_option_.first_mmap_size_ = 0;

    size_ = 0;
    fd_ = -1;
    data_ = NULL;
  }
}

bool MMapFile::sync_file()
{
  if (NULL != data_ && size_ > 0)
  {
    //use MS_ASYNC
    return msync(data_, size_, MS_ASYNC) == 0;
  }
  return true;
}

bool MMapFile::map_file(const bool write)
{
  int flags = PROT_READ;

  if (write)
  {
    flags |= PROT_WRITE;
  }

  if (fd_ < 0)
  {
    return false;
  }

  if (0 == mmap_file_option_.max_mmap_size_)
  {
    return false;
  }

  if (size_ < mmap_file_option_.max_mmap_size_)
  {
    size_ = mmap_file_option_.first_mmap_size_;
  }
  else
  {
    size_ = mmap_file_option_.max_mmap_size_;
  }

  if (!ensure_file_size(size_))
  {
    LOG(ERROR, "ensure file size failed in mremap. size: %d", size_);
    return false;
  }

  data_ = mmap(0, size_, flags | PROT_EXEC, MAP_SHARED, fd_, 0);

  if (MAP_FAILED == data_)
  {
    LOG(ERROR, "map file failed: %s", strerror(errno));
    size_ = 0;
    fd_ = -1;
    data_ = NULL;
    return false;
  };

  LOG(INFO, "mmap file successed, fd: %d maped size: %d, data: %p", fd_, size_, data_);

  return true;
}

bool MMapFile::remap_file()
{
  LOG(INFO, "map file need remap. size: %d, fd: %d", size_, fd_);
  if (fd_ < 0 || data_ == NULL)
  {
    LOG(INFO, "mremap not mapped yet");
    return false;
  }

  if (size_ == mmap_file_option_.max_mmap_size_)
  {
    LOG(INFO, "already mapped max size, now size: %d, max size: %d", size_, mmap_file_option_.max_mmap_size_);
    return false;
  }

  int32_t new_size = size_ + mmap_file_option_.per_mmap_size_;
  if (new_size > mmap_file_option_.max_mmap_size_)
    new_size = mmap_file_option_.max_mmap_size_;

  if (!ensure_file_size(new_size))
  {
    LOG(ERROR, "ensure file size failed in mremap. new size: %d", new_size);
    return false;
  }

  LOG(INFO, "mremap start. fd: %d, now size: %d, new size: %d, old data: %p", fd_, size_, new_size, data_);
  void* new_map_data = mremap(data_, size_, new_size, MREMAP_MAYMOVE);
  if (MAP_FAILED == new_map_data)
  {
    LOG(ERROR, "ensure file size failed in mremap. new size: %d, error desc: %s", new_size, strerror(errno));
    return false;
  }
  else
  {
    LOG(INFO, "mremap success. fd: %d, now size: %d, new size: %d, old data: %p, new data: %p", fd_, size_, new_size,
        data_, new_map_data);
  }

  data_ = new_map_data;
  size_ = new_size;
  return true;
}

void* MMapFile::get_data() const
{
  return data_;
}

int32_t MMapFile::get_size() const
{
  return size_;
}

bool MMapFile::munmap_file()
{
  if (munmap(data_, size_) == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool MMapFile::ensure_file_size(const int32_t size)
{
  struct stat s;
  if (fstat(fd_, &s) < 0)
  {
    LOG(ERROR, "fstat error, error desc: %s", strerror(errno));
    return false;
  }
  if (s.st_size < size)
  {
    if (ftruncate(fd_, size) < 0)
    {
      LOG(ERROR, "ftruncate error, size: %d, error desc: %s", size, strerror(errno));
      return false;
    }
  }

  return true;
}

} //namespace dfs
} //namespace neptune
