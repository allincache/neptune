#ifndef N_DFS_MESSAGE_RENAMEFILEMESSAGE_H
#define N_DFS_MESSAGE_RENAMEFILEMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class RenameFileMessage: public BasePacket
{
public:
  RenameFileMessage();
  virtual ~RenameFileMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline void set_block_id(const uint32_t block_id)
  {
    rename_file_info_.block_id_ = block_id;
  }
  inline uint32_t get_block_id() const
  {
    return rename_file_info_.block_id_;
  }
  inline void set_file_id(const uint64_t file_id)
  {
    rename_file_info_.file_id_ = file_id;
  }
  inline uint64_t get_file_id() const
  {
    return rename_file_info_.file_id_;
  }
  inline void set_new_file_id(const uint64_t file_id)
  {
    rename_file_info_.new_file_id_ = file_id;
  }
  inline uint64_t get_new_file_id() const
  {
    return rename_file_info_.new_file_id_;
  }
  inline void set_ds_list(const VUINT64 &ds)
  {
    ds_ = ds;
  }
  inline const VUINT64& get_ds_list() const
  {
    return ds_;
  }
  inline void set_option_flag(const int32_t flag)
  {
    option_flag_ = flag;
  }
  inline int32_t get_option_flag() const
  {
    return option_flag_;
  }
  inline void set_server()
  {
    rename_file_info_.is_server_ = Slave_Server_Role;
  }
  inline ServerRole is_server() const
  {
    return rename_file_info_.is_server_;
  }
  inline bool has_lease() const
  {
    return (lease_id_ != INVALID_LEASE_ID);
  }
protected:
  RenameFileInfo rename_file_info_;
  int32_t option_flag_;
  mutable VUINT64 ds_;
  mutable int32_t version_;
  mutable uint32_t lease_id_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_RENAMEFILEMESSAGE_H
