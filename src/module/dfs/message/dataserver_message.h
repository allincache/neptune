#ifndef N_DFS_MESSAGE_DATASERVERMESSAGE_H
#define N_DFS_MESSAGE_DATASERVERMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class SetDataserverMessage: public BasePacket
{
 public:
  SetDataserverMessage();
  virtual ~SetDataserverMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  void set_ds(DataServerStatInfo* ds);
  inline void set_has_block(const HasBlockFlag has_block)
  {
    has_block_ = has_block;
  }
  void add_block(BlockInfo* block_info);
  inline HasBlockFlag get_has_block() const
  {
    return has_block_;
  }
  inline const DataServerStatInfo& get_ds() const
  {
    return ds_;
  }
  inline BLOCK_INFO_LIST& get_blocks()
  {
    return blocks_;
  }
 protected:
  DataServerStatInfo ds_;
  BLOCK_INFO_LIST blocks_;
  HasBlockFlag has_block_;
  int8_t heart_interval_;
};

class CallDsReportBlockRequestMessage: public BasePacket
{
 public:
  CallDsReportBlockRequestMessage();
  virtual ~CallDsReportBlockRequestMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline uint64_t get_server() const
  {
    return server_;
  }
  inline void set_server(const uint64_t server)
  {
    server_ = server;
  }
 private:
  uint64_t server_;
};

class ReportBlocksToNsRequestMessage: public BasePacket
{
 public:
  ReportBlocksToNsRequestMessage();
  virtual ~ReportBlocksToNsRequestMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_server(const uint64_t server)
  {
    server_ = server;
  }
  inline uint64_t get_server(void) const
  {
    return server_;
  }
  inline std::set<BlockInfo>& get_blocks()
  {
    return blocks_;
  }

 protected:
  std::set<BlockInfo> blocks_;
  uint64_t server_;
};

class ReportBlocksToNsResponseMessage: public BasePacket
{
 public:
  ReportBlocksToNsResponseMessage();
  virtual ~ReportBlocksToNsResponseMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline std::vector<uint32_t>& get_blocks()
  {
    return expire_blocks_;
  }
  inline void set_server(const uint64_t server)
  {
    server_ = server;
  }
  inline uint64_t get_server(void) const
  {
    return server_;
  }
  inline void set_status(const int8_t status)
  {
    status_ = status;
  }
  inline int8_t get_status(void) const
  {
    return status_;
  }

 protected:
  std::vector<uint32_t> expire_blocks_;
  uint64_t server_;
  int8_t status_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_DATASERVERMESSAGE_H
