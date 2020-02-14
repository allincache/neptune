#ifndef N_DFS_MESSAGE_CLOSEFILEMESSAGE_H
#define N_DFS_MESSAGE_CLOSEFILEMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class CloseFileMessage: public BasePacket
{
 public:
  CloseFileMessage();
  virtual ~CloseFileMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline void set_block_id(const uint32_t block_id)
  {
    close_file_info_.block_id_ = block_id;
  }
  inline void set_file_id(const uint64_t file_id)
  {
    close_file_info_.file_id_ = file_id;
  }
  inline void set_mode(const CloseFileServer mode)
  {
    close_file_info_.mode_ = mode;
  }
  inline void set_crc(const uint32_t crc)
  {
    close_file_info_.crc_ = crc;
  }
  inline void set_file_number(const uint64_t file_number)
  {
    close_file_info_.file_number_ = file_number;
  }
  inline void set_ds_list(const VUINT64& ds)
  {
    ds_ = ds;
  }
  inline void set_block(BlockInfo* const block_info)
  {
    if (NULL != block_info)
      block_ = *block_info;
  }
  inline void set_file_info(FileInfo* const file_info)
  {
    if (NULL != file_info)
      file_info_ = *file_info;
  }
  inline void set_option_flag(const int32_t flag)
  {
    option_flag_ = flag;
  }
  inline void set_block_version(const int32_t version)
  {
    version_ = version;
  }
  inline void set_lease_id(const uint32_t lease_id)
  {
    lease_id_ = lease_id;
  }
  inline uint32_t get_block_id() const
  {
    return close_file_info_.block_id_;
  }
  inline uint64_t get_file_id() const
  {
    return close_file_info_.file_id_;
  }
  inline CloseFileServer get_mode() const
  {
    return close_file_info_.mode_;
  }
  inline uint32_t get_crc() const
  {
    return close_file_info_.crc_;
  }
  inline uint64_t get_file_number() const
  {
    return close_file_info_.file_number_;
  }
  inline const VUINT64& get_ds_list() const
  {
    return ds_;
  }
  inline const BlockInfo* get_block() const
  {
    return block_.block_id_ > 0 ? &block_ : NULL;
  }
  inline const FileInfo* get_file_info() const
  {
    return file_info_.id_ > 0 ? &file_info_ : NULL;
  }
  inline int32_t get_option_flag() const
  {
    return option_flag_;
  }
  inline int32_t get_block_version() const
  {
    return version_;
  }
  inline uint32_t get_lease_id() const
  {
    return lease_id_;
  }
  inline CloseFileInfo get_close_file_info() const
  {
    return close_file_info_;
  }

  inline bool has_lease() const
  {
    return (lease_id_ != INVALID_LEASE_ID);
  }

 protected:
  CloseFileInfo close_file_info_;
  BlockInfo block_;
  FileInfo file_info_;
  mutable VUINT64 ds_;
  int32_t option_flag_;
  mutable int32_t version_;
  mutable uint32_t lease_id_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_CLOSEFILEMESSAGE_H_
