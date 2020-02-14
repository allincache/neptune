#ifndef N_DFS_MESSAGE_UNLINKFILEMESSAGE_H
#define N_DFS_MESSAGE_UNLINKFILEMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

#pragma pack(4)
struct UnlinkFileInfo
{
  int deserialize(const char* data, const int64_t data_len, int64_t& pos);
  int serialize(char* data, const int64_t data_len, int64_t& pos) const;
  int64_t length() const;
  uint32_t block_id_;
  uint64_t file_id_;
  int32_t is_server_;
};
#pragma pack()

class UnlinkFileMessage: public BasePacket
{
 public:
  UnlinkFileMessage();
  virtual ~UnlinkFileMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_block_id(const uint32_t block_id)
  {
    unlink_file_info_.block_id_ = block_id;
  }

  inline uint32_t get_block_id() const
  {
    return unlink_file_info_.block_id_;
  }

  inline void set_file_id(const uint64_t file_id)
  {
    unlink_file_info_.file_id_ = file_id;
  }

  inline uint64_t get_file_id() const
  {
    return unlink_file_info_.file_id_;
  }

  inline void set_ds_list(const VUINT64 &ds)
  {
    dataservers_ = ds;
  }

  inline const VUINT64 &get_ds_list() const
  {
    return dataservers_;
  }

  inline void set_server()
  {
    unlink_file_info_.is_server_ |= 0x1;
  }

  inline int32_t get_server() const
  {
    return unlink_file_info_.is_server_;
  }

  inline void set_unlink_type(const int action)
  {
    unlink_file_info_.is_server_ |= action;
  }

  inline int get_unlink_type() const
  {
    return unlink_file_info_.is_server_ & SYNC;
  }

  inline void set_option_flag(const int32_t flag)
  {
    option_flag_ = flag;
  }

  inline int32_t get_option_flag() const
  {
    return option_flag_;
  }

  inline void set_block_version(const int32_t version)
  {
    version_ = version;
  }

  inline int32_t get_block_version() const
  {
    return version_;
  }

  inline void set_lease_id(const uint32_t lease_id)
  {
    lease_id_ = lease_id;
  }

  inline uint32_t get_lease_id() const
  {
    return lease_id_;
  }

  inline bool is_server() const
  {
    return (unlink_file_info_.is_server_ & 0x1) != 0;
  }

  inline void set_del()
  {
    unlink_file_info_.is_server_ |= DELETE;
  }

  inline void set_undel()
  {
    unlink_file_info_.is_server_ |= UNDELETE;
  }

  inline void set_conceal()
  {
    unlink_file_info_.is_server_ |= CONCEAL;
  }

  inline void set_reveal()
  {
    unlink_file_info_.is_server_ |= REVEAL;
  }

  inline bool has_lease() const
  {
    return (lease_id_ != INVALID_LEASE_ID);
  }

 protected:
  UnlinkFileInfo unlink_file_info_;
  int32_t option_flag_;
  mutable VUINT64 dataservers_;
  mutable int32_t version_;
  mutable uint32_t lease_id_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_UNLINKFILEMESSAGE_H
