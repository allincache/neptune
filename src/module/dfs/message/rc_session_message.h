#ifndef N_DFS_MESSAGE_RCSESSION_MESSAGE_H
#define N_DFS_MESSAGE_RCSESSION_MESSAGE_H

#include "dfs/util/base_packet.h"
#include "dfs/util/rc_define.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class ReqRcLoginMessage: public BasePacket 
{
 public:
  ReqRcLoginMessage();
  virtual ~ReqRcLoginMessage();

  int set_app_key(const char* app_key);
  void set_app_ip(const int64_t app_ip);

  int serialize(Stream& output) const ;
  int deserialize(Stream& input);
  int64_t length() const;

  const char* get_app_key() const;
  int64_t get_app_ip() const;
  void dump() const;

 private:
  char app_key_[MAX_PATH_LENGTH];
  int64_t app_ip_;
  int32_t length_;
};

class RspRcLoginMessage: public BasePacket 
{
 public:
  RspRcLoginMessage();
  virtual ~RspRcLoginMessage();

  int set_session_id(const char* session_id);
  void set_base_info(const BaseInfo& base_info);

  int serialize(Stream& output) const ;
  int deserialize(Stream& input);
  int64_t length() const;

  const char* get_session_id() const;
  const BaseInfo& get_base_info() const;
  void dump() const;

 private:
  char session_id_[MAX_PATH_LENGTH];
  BaseInfo base_info_;
  int32_t length_;
};

class ReqRcKeepAliveMessage: public BasePacket 
{
 public:
  ReqRcKeepAliveMessage();
  virtual ~ReqRcKeepAliveMessage();

  void set_ka_info(const KeepAliveInfo& ka_info);

  int serialize(Stream& output) const ;
  int deserialize(Stream& input);
  int64_t length() const;

  const KeepAliveInfo& get_ka_info() const;
  void dump() const;

 protected:
  KeepAliveInfo ka_info_;
};

class RspRcKeepAliveMessage: public BasePacket 
{
 public:
  RspRcKeepAliveMessage();
  virtual ~RspRcKeepAliveMessage();

  void set_update_flag(const bool update_flag = KA_FLAG);
  void set_base_info(const BaseInfo& base_info);

  int serialize(Stream& output) const ;
  int deserialize(Stream& input);
  int64_t length() const;

  bool get_update_flag() const;
  const BaseInfo& get_base_info() const;
  void dump() const;

 private:
  bool update_flag_;
  BaseInfo base_info_;
};

class ReqRcLogoutMessage: public ReqRcKeepAliveMessage
{
 public:
  ReqRcLogoutMessage();
  virtual ~ReqRcLogoutMessage();
};

//Rsp use status msg 
class ReqRcReloadMessage: public BasePacket 
{
 public:
  ReqRcReloadMessage();
  virtual ~ReqRcReloadMessage();

  void set_reload_type(const ReloadType type);

  int serialize(Stream& output) const ;
  int deserialize(Stream& input);
  int64_t length() const;

  ReloadType get_reload_type() const;
  void dump() const;

 protected:
  ReloadType reload_type_;
};

} //namespace dfs
} //namespace neptune

#endif //DFS_MESSAGE_RCSESSION_MESSAGE_H_
