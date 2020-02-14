#ifndef N_DFS_MESSAGE_GET_DATASERVER_INFORMATION_H
#define N_DFS_MESSAGE_GET_DATASERVER_INFORMATION_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class GetDataServerInformationMessage: public BasePacket
{
 public:
  GetDataServerInformationMessage();
  virtual ~GetDataServerInformationMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  void set_flag(const int16_t flag) { flag_ = flag;}
  int16_t get_flag() const { return flag_;}
 private:
  int16_t flag_;
};

class GetDataServerInformationResponseMessage: public BasePacket
{
 public:
  GetDataServerInformationResponseMessage();
  virtual ~GetDataServerInformationResponseMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;

  void set_super_block(const SuperBlock& block) { sblock_ = block;}
  void set_dataserver_stat_info(const DataServerStatInfo& info) { info_ = info;}
  const SuperBlock& get_super_block() const { return sblock_;}
  const DataServerStatInfo& get_dataserver_stat_info() const { return info_;}

  int32_t& get_bit_map_element_count() {return  bit_map_element_count_;}

  char* get_data() const { return data_;}

  char* alloc_data(const int64_t length);

 protected:
  SuperBlock sblock_;
  DataServerStatInfo info_;
  int32_t bit_map_element_count_;
  int32_t data_length_;
  char* data_;
  int16_t flag_;
  bool alloc_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_GET_DATASERVER_INFORMATION_H
