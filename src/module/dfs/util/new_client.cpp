#include "base/common/Memory.h"
#include "new_client.h"
#include "base/common/ErrorMsg.h"
#include "base/common/Internal.h"
#include "client_manager.h"
#include "local_packet.h"
#include "status_message.h"
#include "base/network/simple/connectionmanager.h"

using namespace neptune::base;

namespace neptune {
namespace dfs {

NewClient::NewClient(const uint32_t& seq_id)
: callback_(NULL),
  source_msg_(NULL),
  seq_id_(seq_id),
  generate_send_id_(0),
  complete_(false),
  post_packet_complete_(false)
{
}

NewClient::~NewClient()
{
  if (NULL != source_msg_)
  {
    source_msg_->free();
    source_msg_ = NULL;
  }
  RESPONSE_MSG_MAP::iterator iter = success_response_.begin();
  for (; iter != success_response_.end(); ++iter)
  {
    if (NULL != iter->second.second)
    {
      iter->second.second->free();
    }
  }
}

bool NewClient::wait(const int64_t timeout_in_ms)
{
#ifdef DFS_TEST
  Packet* packet = NULL;
  for (uint32_t i = 0 ; i < send_id_sign_.size(); i++)
  {
    success_response_.insert(RESPONSE_MSG_MAP::value_type(send_id_sign_[i].first,
      std::pair<uint64_t, Packet*>(send_id_sign_[i].second, packet)));
  }
#endif
  int64_t timeout_ms = timeout_in_ms;
  bool ret = true;
  if (timeout_ms <= 0)
  {
    timeout_ms = DEFAULT_NETWORK_CALL_TIMEOUT;
    ////LOG(WARN, "timeout_in_ms equal 0, we'll use DEFAULT_NETWORK_CALL_TIMEOUT: %"PRI64_PREFIX"d(ms)",
    //  DEFAULT_NETWORK_CALL_TIMEOUT);
  }
  Monitor<Mutex>::Lock lock(monitor_);
  post_packet_complete_ = true;//post packet complete, call timewait
  uint32_t done_count = success_response_.size() + fail_response_.size();
  complete_ = done_count == send_id_sign_.size();
  if (!complete_)
    ret = monitor_.timedWait(Time::milliSeconds(timeout_ms));
  complete_ = true;
  return ret;
}

bool NewClient::async_wait()
{
  {
    Monitor<Mutex>::Lock lock(monitor_);
    post_packet_complete_ = true;//post packet complete
    uint32_t done_count = success_response_.size() + fail_response_.size();
    complete_ = done_count == send_id_sign_.size();
  }
  bool is_callback = complete_ && NULL != callback_;
  if (is_callback)
  {
    NewClientManager::get_instance().do_async_callback(this);
  }
  return true;
}

int NewClient::post_request(const uint64_t server, Packet* packet, uint8_t& send_id)
{
  int32_t ret = NULL != packet && server > 0 ? SUCCESS : EXIT_INVALID_ARGU;
  if (SUCCESS == ret)
  {
    monitor_.lock();
    send_id = create_send_id(server);
    monitor_.unlock();
    if (send_id > MAX_SEND_ID)
    {
      ret = ERROR;
    }
    else
    {
#ifndef DFS_TEST
      WaitId id;
      id.seq_id_ = seq_id_;
      id.send_id_= send_id;
      Packet* send_msg = NewClientManager::get_instance().clone_packet(packet, 2, false);
      if (NULL == send_msg)
      {
        ////LOG(ERROR, "clone message failure, pcode:%d", packet->getPCode());
        ret = ERROR;
        monitor_.lock();
        destroy_send_id(id);
        monitor_.unlock();
      }
      else
      {
        //dynamic_cast<BasePacket*>(send_msg)->set_auto_free(true);
        bool send_ok = NewClientManager::get_instance().connmgr_->sendPacket(server, send_msg,
            NULL, reinterpret_cast<void*>(*(reinterpret_cast<int32_t*>(&id))));
        if (!send_ok)
        {
          ret = EXIT_SENDMSG_ERROR;
          ////LOG(INFO, "cannot post packet, maybe send queue is full or disconnect.");
          send_msg->free();
          monitor_.lock();
          destroy_send_id(id);
          monitor_.unlock();
        }
      }
#endif
    }
  }
  ////LOG(DEBUG, "send msg to server: %s %s, seq_id: %u, send_id: %d, pcode: %d",
  //           CNetUtil::addrToString(server).c_str(), SUCCESS == ret ? "successful" : "fail",
  //           seq_id_, send_id, packet->getPCode());
  return ret;
}

//post_packet is async version of send_packet. donot wait for response packet.
int NewClient::async_post_request(const std::vector<uint64_t>& servers, Packet* packet, callback_func func, bool save_source_msg)
{
  int32_t ret = !servers.empty() && NULL != packet &&  NULL != func ? SUCCESS : EXIT_INVALID_ARGU;
  if (SUCCESS == ret)
  {
    if (NULL == callback_)
    {
      callback_ = func;
    }
    assert (NULL == func || callback_ == func);

    if (save_source_msg && NULL == source_msg_ )
    {
      source_msg_ = NewClientManager::get_instance().clone_packet(packet, DFS_PACKET_VERSION_V2, true);
    }

    std::vector<uint64_t>::const_iterator iter = servers.begin();
    for (; iter != servers.end(); ++iter)
    {
      uint8_t send_id = 0;
      ret = post_request((*iter), packet, send_id);
      if (SUCCESS != ret)
      {
        ////LOG(ERROR, "async post request fail, ret: %d", ret);
        break;
      }
    }
  }

  if (SUCCESS == ret)
  {
    async_wait();
  }
  return ret;
}

bool NewClient::handlePacket(const WaitId& id, Packet* packet, bool& is_callback)
{
  bool ret = true;
  monitor_.lock();
  if (complete_) //wait has return before wakeup. maybe timeout or has reach the wait count
  {
    ret = false;
    //LOG(ERROR, "waitobject has returned before wakeup, waitid: %u, send id: %hhu", id.seq_id_, id.send_id_);
  }
  else
  {
    SEND_SIGN_PAIR* send_id = find_send_id(id);
    if (NULL == send_id)
    {
      ret = false;
      //LOG(ERROR, "'WaitId' object not found by seq_id: %u send_id: %hhu", id.seq_id_, id.send_id_);
    }
    else
    {
      if (packet != NULL && packet->isRegularPacket())
      {
        ret = push_success_response(send_id->first, send_id->second, dynamic_cast<Packet*>(packet));
      }
      else
      {
        ret = push_fail_response(send_id->first, send_id->second);
      }
      uint32_t done_count = success_response_.size() + fail_response_.size();
      complete_ = (done_count == send_id_sign_.size() && post_packet_complete_);
    }
  }
  if (ret && complete_ && NULL == callback_)
  {
    monitor_.notify();
  }
  is_callback = ret && callback_ != NULL && complete_;
  monitor_.unlock();
  return ret;
}

uint8_t NewClient::create_send_id(const uint64_t server)
{
  ++generate_send_id_;
  if (generate_send_id_ > MAX_SEND_ID)
  {
    //LOG(ERROR, "send id: %hhu is invalid, MAX_END_ID: %hhu", generate_send_id_, MAX_SEND_ID);
  }
  else
  {
    send_id_sign_.insert(send_id_sign_.end(), SEND_SIGN_PAIR(generate_send_id_, server));
  }
  return generate_send_id_;
}

bool NewClient::destroy_send_id(const WaitId& id)
{
  std::vector<SEND_SIGN_PAIR>::iterator iter = send_id_sign_.begin();
  for (; iter != send_id_sign_.end(); ++iter)
  {
    if ((*iter).first == id.send_id_)
    {
      send_id_sign_.erase(iter);
      break;
    }
  }
  return true;
}

NewClient::SEND_SIGN_PAIR* NewClient::find_send_id(const WaitId& id)
{
  SEND_SIGN_PAIR* result = NULL;
  std::vector<SEND_SIGN_PAIR>::iterator iter = send_id_sign_.begin();
  for (; iter != send_id_sign_.end(); ++iter)
  {
    if ((*iter).first == id.send_id_)
    {
      result = &(*iter);
      break;
    }
  }
  return result;
}

bool NewClient::push_success_response(const uint8_t send_id, const uint64_t server, Packet* packet)
{
  bool bret = true;
  RESPONSE_MSG_MAP_ITER iter = fail_response_.find(send_id);
  if (iter != fail_response_.end())//erase controlpacket packet
  {
    fail_response_.erase(iter);
  }
  iter = success_response_.find(send_id);
  if (iter == success_response_.end())
  {
    success_response_.insert(std::pair<uint8_t, std::pair<uint64_t, Packet*> >(send_id, std::pair<uint64_t, Packet*>(server, packet)));
  }
  else
  {
    bret = false;
  }
  return bret;
}

bool NewClient::push_fail_response(const uint8_t send_id, const uint64_t server)
{
  bool bret = true;
  RESPONSE_MSG_MAP_ITER iter = success_response_.find(send_id);
  if (iter == success_response_.end())//data packet not found
  {
    iter = fail_response_.find(send_id);
    if (iter == fail_response_.end())
    {
      fail_response_.insert(std::pair<uint8_t, std::pair<uint64_t, Packet*> >(send_id, std::pair<uint64_t, Packet*>(server, NULL)));
    }
  }
  return bret;
}

int send_msg_to_server(uint64_t server, Packet* message, int32_t& status, const int64_t timeout)
{
  NewClient* client = NewClientManager::get_instance().create_client();
  Packet* rmsg = NULL;
  int iret = send_msg_to_server(server, client, message, rmsg, timeout);
  if (SUCCESS == iret)
  {
    iret = rmsg->getPCode() == STATUS_MESSAGE ? SUCCESS : ERROR;
    if (SUCCESS == iret)
    {
      StatusMessage* smsg = dynamic_cast<StatusMessage*>(rmsg);
      status = smsg->get_status();
    }
  }
  NewClientManager::get_instance().destroy_client(client);
  return iret;
}

int send_msg_to_server(uint64_t server, NewClient* client, Packet* msg, Packet*& output/*not free*/, const int64_t timeout)
{
  int32_t iret = NULL != client && server > 0 && NULL != msg && NULL == output ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    uint8_t send_id = 0;
    iret = client->post_request(server, msg, send_id);
    if (SUCCESS == iret)
    {
      client->wait(timeout);
      NewClient::RESPONSE_MSG_MAP* sresponse = client->get_success_response();
      NewClient::RESPONSE_MSG_MAP* fresponse = client->get_fail_response();
      iret = NULL != sresponse && NULL != fresponse ? SUCCESS : ERROR;
      if (SUCCESS == iret)
      {
        iret = sresponse->empty() ? EXIT_TIMEOUT_ERROR : SUCCESS;
        if (SUCCESS == iret)
        {
          NewClient::RESPONSE_MSG_MAP_ITER  iter = sresponse->begin();
          iret = NULL != iter->second.second? SUCCESS : ERROR;
          if (SUCCESS == iret)
          {
            output = iter->second.second;
          }
        }
      }
    }
  }
  return iret;
}

int post_msg_to_server(const std::vector<uint64_t>& servers, NewClient* client, Packet* msg,
                      NewClient::callback_func func, const bool save_msg)
{
  int32_t iret = !servers.empty() && NULL != client && NULL != msg  && NULL != func? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = client->async_post_request(servers, msg, func, save_msg);
  }
  return iret;
}

int post_msg_to_server(uint64_t servers, NewClient* client, Packet* msg,
                      NewClient::callback_func func, const bool save_msg)
{
  int32_t iret = servers > 0 && NULL != client && NULL != msg  && NULL != func? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    std::vector<uint64_t> tmp;
    tmp.push_back(servers);
    iret = client->async_post_request(tmp, msg, func, save_msg);
  }
  return iret;
}

// test whether the DataServerStatInfo is still alive.
int test_server_alive(const uint64_t server_id, const int64_t)
{
  int32_t ret = SUCCESS;
  NewClient* client = NewClientManager::get_instance().create_client();
  if (NULL == client)
  {
    //LOG(ERROR, "%s", "create new client fail");
    ret = ERROR;
  }
  else
  {
    uint8_t send_id = 0;
    StatusMessage send_msg(STATUS_MESSAGE_PING);
    ret = client->post_request(server_id, &send_msg, send_id);
    if (SUCCESS != ret)
    {
      //LOG(ERROR, "test server alive ping server: %s error, server is down",
      //  CNetUtil::addrToString(server_id).c_str());
    }
    else
    {
      bool bret = client->wait();
      if (!bret)
      {
        ret = ERROR;
        //LOG(ERROR,"new client wait server: %s response fail",
        //  CNetUtil::addrToString(server_id).c_str());
      }
      else
      {
        NewClient::RESPONSE_MSG_MAP* response = client->get_success_response();
        if (NULL == response
            || response->empty())
        {
          ret = ERROR;
          //LOG(ERROR, "test server alive ping server: %s error, server mybe down",
          //    CNetUtil::addrToString(server_id).c_str());
        }
        else
        {
          ret = response->begin()->second.second->getPCode() == STATUS_MESSAGE
              ? SUCCESS : ERROR;
        }
      }
    }
  }
  NewClientManager::get_instance().destroy_client(client);
  return ret;
}

} //namespace dfs
} //namespace neptune
