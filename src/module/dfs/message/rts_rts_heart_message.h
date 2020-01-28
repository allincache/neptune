#ifndef N_DFS_MESSAGE_RTS_RTS_HEARTMESSAGE_H
#define N_DFS_MESSAGE_RTS_RTS_HEARTMESSAGE_H

#include "dfs/util/rts_define.h"
#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class RtsRsHeartMessage: public BasePacket 
{
 public:
  RtsRsHeartMessage();
  virtual ~RtsRsHeartMessage();

  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline RootServerInformation& get_rs(void) { return server_;}
  inline void set_rs(const RootServerInformation& server){ memcpy(&server_, &server, sizeof(RootServerInformation));}
  inline int8_t get_type(void) const { return keepalive_type_;}
  inline void set_type(const int8_t type) { keepalive_type_ = type;}
 
 private:
  RootServerInformation server_;
  int8_t keepalive_type_;
};

class RtsRsHeartResponseMessage: public BasePacket 
{
 public:
  RtsRsHeartResponseMessage();
  virtual ~RtsRsHeartResponseMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_active_table_version(const int64_t version) { active_table_version_ = version;}
  inline int64_t get_active_table_version(void) const { return active_table_version_;}
  inline void set_lease_expired_time(const int64_t time) { lease_expired_time_ = time;}
  inline int64_t get_lease_expired_time(void) const { return lease_expired_time_;}
  inline void set_ret_value(const int32_t value) { ret_value_ = value;}
  inline int32_t get_ret_value(void) const { return ret_value_;}
  inline void set_renew_lease_interval_time(const int32_t time) { renew_lease_interval_time_ = time;}
  inline int32_t get_renew_lease_interval_time(void) const { return renew_lease_interval_time_;}
 
 private:
  int64_t active_table_version_;
  int64_t lease_expired_time_;
  int32_t ret_value_;
  int32_t renew_lease_interval_time_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_RTS_RTS_HEARTMESSAGE_H
