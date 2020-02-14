#ifndef N_DFS_MESSAGE_CREATEFILENAMEMESSAGE_H
#define N_DFS_MESSAGE_CREATEFILENAMEMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class CreateFilenameMessage: public BasePacket 
{
 public:
  CreateFilenameMessage();
  virtual ~CreateFilenameMessage();
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
 protected:
  uint32_t block_id_;
  uint64_t file_id_;
};

class RespCreateFilenameMessage: public BasePacket 
{
 public:
  RespCreateFilenameMessage();
  virtual ~RespCreateFilenameMessage();
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
  inline void set_file_number(const uint64_t file_number)
  {
    file_number_ = file_number;
  }
  inline uint64_t get_file_number() const
  {
    return file_number_;
  }

 protected:
  uint32_t block_id_;
  uint64_t file_id_;
  uint64_t file_number_;
};

} //namespace dfs
} //namespace neptune

#endif
