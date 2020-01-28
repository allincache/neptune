#ifndef N_DFS_MESSAGE_WRITEDATAMESSAGE_H
#define N_DFS_MESSAGE_WRITEDATAMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class WriteDataMessage:  public BasePacket
{
 public:
  WriteDataMessage();
  virtual ~WriteDataMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline void set_block_id(const uint32_t block_id)
  {
    write_data_info_.block_id_ = block_id;
  }
  inline uint32_t get_block_id() const
  {
    return write_data_info_.block_id_;
  }
  inline void set_file_id(const uint64_t file_id)
  {
    write_data_info_.file_id_ = file_id;
  }
  inline uint64_t get_file_id() const
  {
    return write_data_info_.file_id_;
  }
  inline void set_offset(const int32_t offset)
  {
    write_data_info_.offset_ = offset;
  }
  inline int32_t get_offset() const
  {
    return write_data_info_.offset_;
  }
  inline void set_length(const int32_t length)
  {
    write_data_info_.length_ = length;
  }
  inline int32_t get_length() const
  {
    return write_data_info_.length_;
  }
  inline void set_data(char* const data)
  {
    data_ = data;
  }
  inline const char* const get_data() const
  {
    return data_;
  }
  inline void set_ds_list(const VUINT64 &ds)
  {
    ds_ = ds;
  }
  inline VUINT64 &get_ds_list()
  {
    return ds_;
  }
  inline void set_server(const ServerRole is_server)
  {
    write_data_info_.is_server_ = is_server;
  }
  inline ServerRole get_server() const
  {
    return write_data_info_.is_server_;
  }
  inline void set_file_number(const uint64_t file_num)
  {
    write_data_info_.file_number_ = file_num;
  }
  inline uint64_t get_file_number() const
  {
    return write_data_info_.file_number_;
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
  inline WriteDataInfo get_write_info() const
  {
    return write_data_info_;
  }
  inline uint32_t has_lease() const
  {
    return (lease_id_ != INVALID_LEASE_ID);
  }

 protected:
  WriteDataInfo write_data_info_;
  const char* data_;
  mutable VUINT64 ds_;
  mutable int32_t version_;
  mutable uint32_t lease_id_;
};

#ifdef _DEL_001_
class RespWriteDataMessage :  public BasePacket
{
 public:
  RespWriteDataMessage();
  virtual ~RespWriteDataMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  void set_length(const int32_t length)
  {
    length_ = length;
  }

  int32_t get_length() const
  {
    return length_;
  }

  virtual int parse(char* data, int32_t len);
  virtual int build(char* data, int32_t len);
  virtual int32_t message_length();
  virtual char* get_name();
  protected:
  int32_t length_;
};
#endif

class WriteRawDataMessage:  public BasePacket
{
  public:
    WriteRawDataMessage();
    virtual ~WriteRawDataMessage();
    virtual int serialize(Stream& output) const ;
    virtual int deserialize(Stream& input);
    virtual int64_t length() const;

    inline void set_block_id(const uint32_t block_id)
    {
      write_data_info_.block_id_ = block_id;
    }
    inline uint32_t get_block_id() const
    {
      return write_data_info_.block_id_;
    }
    inline void set_offset(const int32_t offset)
    {
      write_data_info_.offset_ = offset;
    }
    inline int32_t get_offset() const
    {
      return write_data_info_.offset_;
    }
    inline void set_length(const int32_t length)
    {
      write_data_info_.length_ = length;
    }
    inline int32_t get_length() const
    {
      return write_data_info_.length_;
    }
    inline void set_data(const char* data)
    {
      data_ = data;
    }
    inline const char* const get_data() const
    {
      return data_;
    }
    inline void set_new_block(const int32_t flag)
    {
      flag_ = flag;
    }
    inline int32_t get_new_block() const
    {
      return flag_;
    }
  protected:
    WriteDataInfo write_data_info_;
    const char* data_;
    int32_t flag_;
};

class WriteInfoBatchMessage:  public BasePacket
{
 public:
  WriteInfoBatchMessage();
  virtual ~WriteInfoBatchMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  inline void set_block_id(const uint32_t block_id)
  {
    write_data_info_.block_id_ = block_id;
  }
  inline uint32_t get_block_id() const
  {
    return write_data_info_.block_id_;
  }
  inline void set_offset(const int32_t offset)
  {
    write_data_info_.offset_ = offset;
  }
  inline int32_t get_offset() const
  {
    return write_data_info_.offset_;
  }
  inline void set_length(const int32_t length)
  {
    write_data_info_.length_ = length;
  }
  inline int32_t get_length() const
  {
    return write_data_info_.length_;
  }
  inline void set_cluster(const int32_t cluster)
  {
    cluster_ = cluster;
  }
  inline int32_t get_cluster() const
  {
    return cluster_;
  }
  inline void set_raw_meta_list(const RawMetaVec* list)
  {
    if (NULL != list)
      meta_list_ = (*list);
  }
  inline const RawMetaVec* get_raw_meta_list() const
  {
    return &meta_list_;
  }
  inline void set_block_info(BlockInfo* const block_info)
  {
    if (NULL != block_info)
      block_info_ = *block_info;
  }
  inline const BlockInfo* get_block_info() const
  {
    return block_info_.block_id_ <= 0 ? NULL : &block_info_;
  }

 protected:
  WriteDataInfo write_data_info_;
  BlockInfo block_info_;
  RawMetaVec meta_list_;
  int32_t cluster_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_WRITEDATAMESSAGE_H
