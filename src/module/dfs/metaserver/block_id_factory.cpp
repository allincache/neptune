#include "base/common/ErrorMsg.h"
#include "base/fs/DirectoryOp.h"
#include "block_id_factory.h"

namespace neptune {
namespace dfs {
namespace metaserver {

const uint16_t BlockIdFactory::BLOCK_START_NUMBER = 100;
const uint16_t BlockIdFactory::SKIP_BLOCK_NUMBER  = 100;

BlockIdFactory::BlockIdFactory():
  global_id_(BLOCK_START_NUMBER),
  count_(0),
  fd_(-1)
{

}

BlockIdFactory::~BlockIdFactory()
{

}

int BlockIdFactory::initialize(const std::string& path)
{
  int32_t ret = path.empty() ? EXIT_GENERAL_ERROR : SUCCESS;
  if (SUCCESS == ret)
  {
    if (!DirectoryOp::create_full_path(path.c_str()))
    {
      LOG(ERROR, "create directory: %s fail...", path.c_str());
      ret = EXIT_GENERAL_ERROR;
    }
    if (SUCCESS == ret)
    {
      std::string fs_path = path + "/ns.meta";
      fd_ = ::open(fs_path.c_str(), O_RDWR | O_CREAT, 0600);
      if (fd_ < 0)
      {
        LOG(ERROR, "open file %s failed, errors: %s", fs_path.c_str(), strerror(errno));
        ret = EXIT_GENERAL_ERROR;
      }
    }
    if (SUCCESS == ret)
    {
      char data[INT_SIZE];
      int32_t length = ::read(fd_, data, INT_SIZE);
      if (length == INT_SIZE)//read successful
      {
        int64_t pos = 0;
        ret = Serialization::get_int32(data, INT_SIZE, pos, reinterpret_cast<int32_t*>(&global_id_));
        if (SUCCESS != ret)
        {
          LOG(ERROR, "serialize global block id error, ret: %d", ret);
        }
        else
        {
          if (global_id_ < BLOCK_START_NUMBER)
            global_id_ = BLOCK_START_NUMBER;
        }
        if (SUCCESS == ret)
        {
          global_id_ += SKIP_BLOCK_NUMBER;
        }
      }
    }
  }
  return ret;
}

int BlockIdFactory::destroy()
{
  int32_t ret = SUCCESS;
  if (fd_ > 0)
  {
    ret = update(global_id_);
    ::close(fd_);
  }
  return ret;
}

uint32_t BlockIdFactory::generation(const uint32_t id)
{
  bool update_flag = false;
  uint32_t ret_id = INVALID_BLOCK_ID;
  {
    Mutex::Lock lock(mutex_);
    ++count_;
    if (id == 0)
    {
      ret_id = ++global_id_;
    }
    else
    {
      ret_id = id;
      global_id_ = std::max(global_id_, id);
    }
    if (count_ >= SKIP_BLOCK_NUMBER)
    {
      update_flag = true;
      count_ = 0;
    }
  }
  if (update_flag)
  {
    int32_t ret = update(ret_id);
    if (SUCCESS != ret)
    {
      LOG(WARN, "update global block id failed, id: %u, ret: %d", ret_id, ret);
      ret_id = INVALID_BLOCK_ID;
    }
  }
  return ret_id;
}

uint32_t BlockIdFactory::skip(const int32_t num)
{
  mutex_.lock();
  global_id_ += num;
  uint32_t id = global_id_;
  mutex_.unlock();
  int32_t ret = update(id);
  if (SUCCESS != ret)
  {
    LOG(WARN, "update global block id failed, id: %u, ret: %d", id, ret);
  }
  return id;
}

int BlockIdFactory::update(const uint32_t id) const
{
  assert(fd_ != -1);
  char data[INT_SIZE];
  int64_t pos = 0;
  int32_t ret = Serialization::set_int32(data, INT_SIZE, pos, id);
  if (SUCCESS == ret)
  {
    int32_t offset = 0;
    int32_t length = 0;
    int32_t count  = 0;
    ::lseek(fd_, 0, SEEK_SET);
    do
    {
      ++count;
      length = ::write(fd_, (data + offset), (INT_SIZE - offset));
      if (length > 0)
      {
        offset += length;
      }
    }
    while (count < 3 && offset < INT_SIZE);
    ret = INT_SIZE == offset ? SUCCESS : ERROR;
  }
  return ret;
}

}
}
}
