#ifndef N_DFS_MESSAGE_ROLLBACKMESSAGE_H
#define N_DFS_MESSAGE_ROLLBACKMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class RollbackMessage: public BasePacket 
{
 public:
  RollbackMessage();
  virtual ~RollbackMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline int32_t get_act_type() const
  {
    return act_type_;
  }
  inline void set_act_type(const int32_t type)
  {
    act_type_ = type;
  }
  inline const BlockInfo* get_block_info() const
  {
    return &block_info_;
  }
  inline void set_block_info(BlockInfo* const block_info)
  {
    if (NULL != block_info)
      block_info_ = *block_info;
  }
  inline const FileInfo* get_file_info() const
  {
    return &file_info_;
  }
  inline void set_file_info(FileInfo* const file_info) 
  {
    if (NULL != file_info)
      file_info_ = *file_info;
  }
 private:
  int32_t act_type_;
  BlockInfo block_info_;
  FileInfo file_info_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_ROLLBACKMESSAGE_H
