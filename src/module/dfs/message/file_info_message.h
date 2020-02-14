#ifndef N_DFS_MESSAGE_FILEINFOMESSAGE_H
#define N_DFS_MESSAGE_FILEINFOMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class FileInfoMessage: public BasePacket 
{
 public:
  FileInfoMessage();
  virtual ~FileInfoMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_block_id(const uint32_t block_id)
  {
    block_id_ = block_id;
  }
  inline uint32_t get_block_id() const
  {
    return block_id_;
  }
  inline void set_file_id(const uint64_t file_id)
  {
    file_id_ = file_id;
  }
  inline uint64_t get_file_id() const
  {
    return file_id_;
  }
  inline void set_mode(const int32_t mode)
  {
    mode_ = mode;
  }
  inline int32_t get_mode() const
  {
    return mode_;
  }
 protected:
  uint32_t block_id_;
  uint64_t file_id_;
  int32_t mode_;
};

class RespFileInfoMessage: public BasePacket 
{
 public:
  RespFileInfoMessage();
  virtual ~RespFileInfoMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_file_info(FileInfo* const file_info)
  {
    if (NULL != file_info)
    {
      file_info_ = *file_info;
    }
  }
  inline const FileInfo* get_file_info()  const
  {
    return &file_info_;
  }
 protected:
  FileInfo file_info_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_FILEINFOMESSAGE_H
