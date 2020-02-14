#include "dfs/util/parameter.h"
#include "dfs/util/func.h"
#include "data_file.h"
#include "dataservice.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;

DataFile::DataFile(uint64_t fn)
{
  last_update_ = time(NULL);
  length_ = 0;
  crc_ = 0;
  fd_ = -1;
  sprintf(tmp_file_name_, "%s/tmp/%" PRI64_PREFIX "u.dat", dynamic_cast<DataService*>(DataService::instance())->get_real_work_dir().c_str(), fn);
  atomic_set(&ref_count_, 0);
}

DataFile::DataFile(uint64_t fn, char* path)
{
  UNUSED(path);
  last_update_ = time(NULL);
  length_ = 0;
  crc_ = 0;
  fd_ = -1;
  sprintf(tmp_file_name_, "%s/tmp/%" PRI64_PREFIX "u.dat", dynamic_cast<DataService*>(DataService::instance())->get_real_work_dir().c_str(), fn);
  atomic_set(&ref_count_, 0);
}

DataFile::~DataFile()
{
  set_over();
}

// clean over
void DataFile::set_over()
{
  length_ = 0;
  if (fd_ != -1)
  {
    close(fd_);
    unlink(tmp_file_name_);
    fd_ = -1;
  }
}

int DataFile::set_data(const char *data, const int32_t len, const int32_t offset)
{
  if (len <= 0)
  {
    return SUCCESS;
  }
  int32_t length = offset + len;

  if (length > WRITE_DATA_TMPBUF_SIZE || length_ > WRITE_DATA_TMPBUF_SIZE)      // write to file if length is large then max_read_size
  {
    if (fd_ == -1)          // first write to file
    {
      fd_ = open(tmp_file_name_, O_RDWR | O_CREAT | O_TRUNC, 0600);
      if (fd_ == -1)
      {
        //LOG(ERROR, "open file fail: %s, %s", tmp_file_name_, strerror(errno));
        return ERROR;
      }
      // write memory content to file
      if (write(fd_, data_, length_) != length_)
      {
        //LOG(ERROR, "write file fail: %s, length_: %d, error:%s", tmp_file_name_, length_, strerror(errno));
        return ERROR;
      }
    }
    if (lseek(fd_, offset, SEEK_SET) == -1)
    {
      //LOG(ERROR, "lseek file fail: %s, offset: %d", tmp_file_name_, offset);
      return ERROR;
    }
    // no append, just write
    if (write(fd_, data, len) != len)
    {
      //LOG(ERROR, "write file fail: %s, len: %d", tmp_file_name_, len);
      return ERROR;
    }
  }
  else                      // length ok
  {
    memcpy(data_ + offset, data, len);
  }

  // set the max length
  if (length > length_)
  {
    length_ = length;
  }
  return len;
}

char* DataFile::get_data(char* data, int32_t* len, int32_t offset)
{
  if (offset >= length_)
  {
    *len = -1;
    return NULL;
  }
  if (length_ > WRITE_DATA_TMPBUF_SIZE) // read from file
  {
    if (fd_ == -1)
    {
      *len = -1;
      return NULL;
    }
    if (lseek(fd_, offset, SEEK_SET) == -1)
    {
      //LOG(ERROR, "lseek file fail: %s, offset: %d", tmp_file_name_, offset);
      *len = -1;
      return NULL;
    }
    if (data == NULL)
    {
      data = data_;
      *len = WRITE_DATA_TMPBUF_SIZE;
    }
    int rlen;
    if ((rlen = read(fd_, data, *len)) < 0)
    {
      //LOG(ERROR, "read file fail: %s, len: %d", tmp_file_name_, *len);
      *len = -1;
      return NULL;
    }
    *len = rlen;
  }
  else
  {
    if (data == NULL)       // just use inner buffer
    {
      data = data_;
      *len = length_;
    }
    else
    {
      if (*len > length_)
      {
        *len = length_;
      }
      memcpy(data, data_, *len);
    }
  }
  return data;
}

uint32_t DataFile::get_crc()
{
  if (crc_ == 0)
  {
    if (length_ > WRITE_DATA_TMPBUF_SIZE) // read from file
    {
      if (fd_ == -1)
      {
        return crc_;
      }
      if (lseek(fd_, 0, SEEK_SET) == -1)
      {
        return crc_;
      }
      int rlen;
      while ((rlen = read(fd_, data_, WRITE_DATA_TMPBUF_SIZE)) > 0)
      {
        crc_ = Func::crc(crc_, data_, rlen);
      }
    }
    else
    {
      crc_ = Func::crc(0, data_, length_);
    }
  }
  return crc_;
}

}
}
}
