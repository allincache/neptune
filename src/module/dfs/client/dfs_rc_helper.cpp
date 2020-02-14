#include "dfs_rc_helper.h"
#include "dfs/util/rc_define.h"
#include "dfs/util/new_client.h"
#include "dfs/util/client_manager.h"
#include "dfs/util/base_packet.h"
#include "dfs/util/status_message.h"
#include "dfs/message/message_factory.h"
#include "dfs/message/rc_session_message.h"
#include "client_config.h"

using namespace neptune::dfs;
using namespace std;

int RcHelper::login(const uint64_t rc_ip, const string& app_key, const uint64_t app_ip,
    string& session_id, BaseInfo& base_info)
{
  ReqRcLoginMessage req_login_msg;
  req_login_msg.set_app_key(app_key.c_str());
  req_login_msg.set_app_ip(app_ip);

  Packet* rsp = NULL;
  NewClient* client = NewClientManager::get_instance().create_client();
  int ret = send_msg_to_server(rc_ip, client, &req_login_msg, rsp, ClientConfig::wait_timeout_);
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "call req login rc fail, rc_ip: %"PRI64_PREFIX"u, app_key: %s, app_ip: %"PRI64_PREFIX"u, ret: %d",
    //    rc_ip, app_key.c_str(), app_ip, ret);
  }
  else if (RSP_RC_LOGIN_MESSAGE == rsp->getPCode()) //rsp will not be null
  {
    RspRcLoginMessage* rsp_login_msg = dynamic_cast<RspRcLoginMessage*>(rsp);

    session_id = rsp_login_msg->get_session_id();
    base_info = rsp_login_msg->get_base_info();
  }
  else
  {
    ret = EXIT_UNKNOWN_MSGTYPE;
    if (STATUS_MESSAGE == rsp->getPCode())
    {
      //LOG(ERROR, "call req login rc fail, rc_ip: %"PRI64_PREFIX"u, app_key: %s, app_ip: %"PRI64_PREFIX"u,"
      //    "ret: %d, error: %s, status: %d",
      //    rc_ip, app_key.c_str(), app_ip, ret,
      //    dynamic_cast<StatusMessage*>(rsp)->get_error(), dynamic_cast<StatusMessage*>(rsp)->get_status());
    }
    else
    {
      //LOG(ERROR, "call req login rc fail, rc_ip: %"PRI64_PREFIX"u, app_key: %s, app_ip: %"PRI64_PREFIX"u, ret: %d, msg type: %d",
      //    rc_ip, app_key.c_str(), app_ip, ret, rsp->getPCode());
    }
  }
  NewClientManager::get_instance().destroy_client(client);
  return ret;
}

int RcHelper::keep_alive(const uint64_t rc_ip, const KeepAliveInfo& ka_info,
    bool& update_flag, BaseInfo& base_info)
{
  ReqRcKeepAliveMessage req_ka_msg;
  req_ka_msg.set_ka_info(ka_info);

  Packet* rsp = NULL;
  NewClient* client = NewClientManager::get_instance().create_client();
  int ret = send_msg_to_server(rc_ip, client, &req_ka_msg, rsp, ClientConfig::wait_timeout_);
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "call req keep alive rc fail, rc_ip: %"PRI64_PREFIX"u, session_id: %s, ret: %d",
    //    rc_ip, ka_info.s_base_info_.session_id_.c_str(), ret);
  }
  else if (RSP_RC_KEEPALIVE_MESSAGE == rsp->getPCode()) //rsp will not be null
  {
    RspRcKeepAliveMessage* rsp_ka_msg = dynamic_cast<RspRcKeepAliveMessage*>(rsp);

    update_flag = rsp_ka_msg->get_update_flag();
    if (update_flag)
    {
      base_info = rsp_ka_msg->get_base_info();
    }
  }
  else
  {
    ret = EXIT_UNKNOWN_MSGTYPE;
    if (STATUS_MESSAGE == rsp->getPCode())
    {
      //LOG(ERROR, "call req keep alive rc fail, rc_ip: %"PRI64_PREFIX"u, session_id: %s, ret: %d, error: %s, status: %d",
      //    rc_ip, ka_info.s_base_info_.session_id_.c_str(), ret,
      //    dynamic_cast<StatusMessage*>(rsp)->get_error(), dynamic_cast<StatusMessage*>(rsp)->get_status());
    }
    else
    {
      //LOG(ERROR, "call req keep alive rc fail, rc_ip: %"PRI64_PREFIX"u, session_id: %s, ret: %d, msg type: %d",
      //    rc_ip, ka_info.s_base_info_.session_id_.c_str(), ret, rsp->getPCode());
    }
  }
  NewClientManager::get_instance().destroy_client(client);
  return ret;
}

int RcHelper::logout(const uint64_t rc_ip, const KeepAliveInfo& ka_info)
{
  ReqRcLogoutMessage req_logout_msg;
  req_logout_msg.set_ka_info(ka_info);

  Packet* rsp = NULL;
  NewClient* client = NewClientManager::get_instance().create_client();
  int ret = send_msg_to_server(rc_ip, client, &req_logout_msg, rsp, ClientConfig::wait_timeout_);
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "call req logout rc fail, rc_ip: %"PRI64_PREFIX"u, session_id: %s, ret: %d",
    //    rc_ip, ka_info.s_base_info_.session_id_.c_str(), ret);
  }
  else if (STATUS_MESSAGE == rsp->getPCode()) //rsp will not be null
  {
    if (STATUS_MESSAGE_OK != dynamic_cast<StatusMessage*>(rsp)->get_status())
    {
      ret = dynamic_cast<StatusMessage*>(rsp)->get_status();
      //LOG(ERROR, "call req logout rc fail, rc_ip: %"PRI64_PREFIX"u, session_id: %s, ret: %d",
      //    rc_ip, ka_info.s_base_info_.session_id_.c_str(), ret);
    }
  }
  else
  {
    ret = EXIT_UNKNOWN_MSGTYPE;
    //LOG(ERROR, "call req logout rc fail, rc_ip: %"PRI64_PREFIX"u, session_id: %s, ret: %d, msg type: %d",
    //    rc_ip, ka_info.s_base_info_.session_id_.c_str(), ret, rsp->getPCode());
  }
  NewClientManager::get_instance().destroy_client(client);
  return ret;
}

int RcHelper::reload(const uint64_t rc_ip, const ReloadType reload_type)
{
  ReqRcReloadMessage req_reload_msg;
  req_reload_msg.set_reload_type(reload_type);

  Packet* rsp = NULL;
  NewClient* client = NewClientManager::get_instance().create_client();
  int ret = send_msg_to_server(rc_ip, client, &req_reload_msg, rsp, ClientConfig::wait_timeout_);
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "call req reload rc fail, rc_ip: %"PRI64_PREFIX"u, reload_type: %d, ret: %d",
    //    rc_ip, reload_type, ret);
  }
  else if (STATUS_MESSAGE == rsp->getPCode()) //rsp will not be null
  {
    if (STATUS_MESSAGE_OK != dynamic_cast<StatusMessage*>(rsp)->get_status())
    {
      ret = dynamic_cast<StatusMessage*>(rsp)->get_status();
      //LOG(ERROR, "call req reload rc fail, rc_ip: %"PRI64_PREFIX"u, reload_type: %d, ret: %d",
      //    rc_ip, reload_type, ret);
    }
  }
  else
  {
    ret = EXIT_UNKNOWN_MSGTYPE;
    //LOG(ERROR, "call req reload rc fail, rc_ip: %"PRI64_PREFIX"u, reload_type: %d, ret: %d, msg type: %d",
    //    rc_ip, reload_type, ret, rsp->getPCode());
  }
  NewClientManager::get_instance().destroy_client(client);
  return ret;
}
