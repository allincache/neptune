#include "dfs/util/dfs.h"
#include "base/common/Memory.h"
#include "base/concurrent/Mutex.h"
#include "ns_define.h"
#include "metaserver.h"
#include "base/common/ErrorMsg.h"
#include "dfs/util/config_item.h"
#include "dfs/util/client_manager.h"
#include "heart_manager.h"
#include "global_factory.h"

using namespace neptune::dfs;

namespace neptune {
namespace dfs {
namespace metaserver {

HeartManagement::HeartManagement(MetaServer& m) :
  manager_(m),
  keepalive_queue_header_(*this),
  report_block_queue_header_(*this)
{

}

HeartManagement::~HeartManagement()
{

}

int HeartManagement::initialize(const int32_t keepalive_thread_count,const int32_t report_block_thread_count)
{
  keepalive_threads_.setThreadParameter(keepalive_thread_count, &keepalive_queue_header_, this);
  report_block_threads_.setThreadParameter(report_block_thread_count, &report_block_queue_header_, this);
  keepalive_threads_.start();
  report_block_threads_.start();
  return SUCCESS;
}

void HeartManagement::wait_for_shut_down()
{
  keepalive_threads_.wait();
  report_block_threads_.wait();
}

void HeartManagement::destroy()
{
  keepalive_threads_.stop(true);
  report_block_threads_.stop(true);
}

/**
 * push do lot of things.
 * first call base_type::push check if current processing queue size > max_queue_size_
 * if true cannot processing this heart message, directly response to client with busy repsonse.
 * pay special attention to free the message...
 */
int HeartManagement::push(BasePacket* msg)
{
  int32_t ret = (NULL != msg) ? SUCCESS : EXIT_PARAMETER_ERROR;
  if (SUCCESS == ret)
  {
    bool handled = false;
    int32_t pcode = msg->getPCode();
    int32_t status = 0;
    uint64_t server = 0;
    //normal or login or logout heartbeat message, just push, cannot blocking!
    if (pcode == SET_DATASERVER_MESSAGE)
    {
      SetDataserverMessage* message = dynamic_cast<SetDataserverMessage*>(msg);
      server = message->get_ds().id_;
      status = message->get_ds().status_;
      handled = keepalive_threads_.push(msg, SYSPARAM_NAMESERVER.keepalive_queue_size_, false);
    }
    else if (pcode == REQ_REPORT_BLOCKS_TO_NS_MESSAGE)
    {
      //dataserver report block heartbeat message, cannot blocking!
      ReportBlocksToNsRequestMessage* message = dynamic_cast<ReportBlocksToNsRequestMessage*>(msg);
      server = message->get_server();
      handled = report_block_threads_.push(msg, SYSPARAM_NAMESERVER.report_block_queue_size_, false);
    }
    else
    {
      //LOG(INFO, "pcode: %d invalid", pcode);
    }
    ret = handled ? SUCCESS : EXIT_GENERAL_ERROR;
    if (SUCCESS != ret)
    {
      //threadpool busy..cannot handle it
      //ret = msg->reply_error_packet(//LOG_LEVEL(WARN), STATUS_MESSAGE_ERROR,
      //    "metaserver.heartbeat busy! cannot accept this request from : %s, type: %s, status: %d",
      //    CNetUtil::addrToString(server).c_str(),
      //    pcode == SET_DATASERVER_MESSAGE ? "heartbeat" :
      //    pcode == REQ_REPORT_BLOCKS_TO_NS_MESSAGE ? "report block" : "unknown", status);
      // already repsonse, now can free this message object.
      msg->free();
    }
  }
  return ret;
}

// event handler
bool HeartManagement::KeepAliveIPacketQueueHeaderHelper::handlePacketQueue(Packet *packet, void *args)
{
  UNUSED(args);
  bool bret = (packet != NULL);
  if (bret)
  {
    //if return SUCCESS, packet had been delete in this func
    //if handlePacketQueue return true, tbnet will delete this packet
    manager_.keepalive(packet);
  }
  return bret;
}

bool HeartManagement::ReportBlockIPacketQueueHeaderHelper::handlePacketQueue(Packet *packet, void *args)
{
  UNUSED(args);
  bool bret = (packet != NULL);
  if (bret)
  {
    //if return SUCCESS, packet had been delete in this func
    //if handlePacketQueue return true, tbnet will delete this packet
    manager_.report_block(packet);
  }
  return bret;
}

int HeartManagement::keepalive(Packet* packet)
{
  int32_t ret = (NULL != packet && SET_DATASERVER_MESSAGE == packet->getPCode()) ? SUCCESS : EXIT_PARAMETER_ERROR;
  if (SUCCESS == ret)
  {
    Time begin = Time::now();
    SetDataserverMessage* message = dynamic_cast<SetDataserverMessage*> (packet);
    assert(SET_DATASERVER_MESSAGE == packet->getPCode());
    RespHeartMessage *result_msg = new RespHeartMessage();
    result_msg->set_heart_interval(SYSPARAM_NAMESERVER.heart_interval_);
    const DataServerStatInfo& ds_info = message->get_ds();
    time_t now = Func::get_monotonic_time();

    ret = manager_.get_layout_manager().get_client_request_server().keepalive(ds_info, now);
    result_msg->set_status(SUCCESS == ret ? HEART_MESSAGE_OK : HEART_MESSAGE_FAILED);
    if (SUCCESS == ret
        && DATASERVER_STATUS_DEAD == ds_info.status_)
    {
      //dataserver exit
      //LOG(INFO, "dataserver: %s exit", CNetUtil::addrToString(ds_info.id_).c_str());
    }
    ret = message->reply(result_msg);
    time_t consume = (Time::now() - begin).toMicroSeconds();
    //LOG(DEBUG, "dataserver: %s %s %s consume times: %"PRI64_PREFIX"d(us), ret: %d", CNetUtil::addrToString(ds_info.id_).c_str(),
    //  DATASERVER_STATUS_DEAD == ds_info.status_ ? "exit" : DATASERVER_STATUS_ALIVE  == ds_info.status_ ? "keepalive" :
    //  "unknow", SUCCESS == ret ? "successful" : "failed", consume, ret);
  }
  return ret;
}

int HeartManagement::report_block(Packet* packet)
{
  uint64_t server = 0;
  int32_t block_nums = 0, result = 0;
  time_t  consume = 0;
  int32_t ret = (NULL != packet && REQ_REPORT_BLOCKS_TO_NS_MESSAGE == packet->getPCode()) ? SUCCESS : EXIT_PARAMETER_ERROR;
  if (SUCCESS == ret)
  {
    Time begin = Time::now();
    ReportBlocksToNsRequestMessage* message = dynamic_cast<ReportBlocksToNsRequestMessage*> (packet);
    assert(REQ_REPORT_BLOCKS_TO_NS_MESSAGE == packet->getPCode());
    server = message->get_server();
    NsRuntimeGlobalInformation& ngi = GFactory::get_runtime_info();
    ReportBlocksToNsResponseMessage* result_msg = new ReportBlocksToNsResponseMessage();
    result_msg->set_server(ngi.owner_ip_port_);
    time_t now = Func::get_monotonic_time();
    result = ret = manager_.get_layout_manager().get_client_request_server().report_block(server, now, message->get_blocks());
    result_msg->set_status(HEART_MESSAGE_OK);
    block_nums = message->get_blocks().size();
    consume = (Time::now() - begin).toMicroSeconds();
    ret = message->reply(result_msg);
  }
  //LOG(INFO, "dataserver: %s report block %s, ret: %d, blocks: %d, consume time: %"PRI64_PREFIX"u(us)",
  //   CNetUtil::addrToString(server).c_str(), SUCCESS == ret ? "successful" : "failed",
  //   result , block_nums, consume);
  return ret;
}

MetaServerHeartManager::MetaServerHeartManager(LayoutManager& manager):
  manager_(manager),
  check_thread_(0)
{

}

MetaServerHeartManager::~MetaServerHeartManager()
{

}

int MetaServerHeartManager::initialize()
{
  work_thread_.setThreadParameter(1, this, this);
  work_thread_.start();
  check_thread_ = new CheckThreadHelper(*this);
  return SUCCESS;
}

int MetaServerHeartManager::wait_for_shut_down()
{
  work_thread_.wait();
  if (0 != check_thread_)
    check_thread_->join();
  return SUCCESS;
}

int MetaServerHeartManager::destroy()
{
  work_thread_.stop(true);
  return SUCCESS;
}

int MetaServerHeartManager::push(BasePacket* message, const int32_t max_queue_size, const bool block)
{
  return work_thread_.push(message, max_queue_size, block);
}

bool MetaServerHeartManager::handlePacketQueue(Packet *packet, void *args)
{
  UNUSED(args);
  bool bret = (packet != NULL);
  if (bret)
  {
    BasePacket* message = dynamic_cast<BasePacket*>(packet);
    int32_t ret = SUCCESS;
    if (message->getPCode() == HEARTBEAT_AND_NS_HEART_MESSAGE)
      ret = keepalive_in_heartbeat_(message);
    else
      ret = keepalive_(message);
  }
  return bret;
}

int MetaServerHeartManager::keepalive_(BasePacket* message)
{
  int32_t ret = NULL != message ? SUCCESS : EXIT_PARAMETER_ERROR;
  if (SUCCESS == ret)
  {
    ret = message->getPCode() == MASTER_AND_SLAVE_HEART_MESSAGE ? SUCCESS : EXIT_UNKNOWN_MSGTYPE;
    if (SUCCESS == ret)
    {
      MasterAndSlaveHeartMessage* msg = dynamic_cast<MasterAndSlaveHeartMessage*>(message);
      NsRuntimeGlobalInformation& ngi = GFactory::get_runtime_info();
      bool login = msg->get_type() == NS_KEEPALIVE_TYPE_LOGIN;
      bool get_peer_role = msg->get_flags() == HEART_GET_PEER_ROLE_FLAG_YES;
      int64_t lease_id = msg->get_lease_id();
      MasterAndSlaveHeartResponseMessage* reply_msg = new MasterAndSlaveHeartResponseMessage();
      if (get_peer_role)
      {
        reply_msg->set_ip_port(ngi.owner_ip_port_);
        reply_msg->set_role(ngi.owner_role_);
        reply_msg->set_status(ngi.owner_status_);
      }
      else
      {
        time_t now = Func::get_monotonic_time();
        ret = ngi.keepalive(lease_id, msg->get_ip_port(), msg->get_role(),
                  msg->get_status(), msg->get_type(), now);
        reply_msg->set_ip_port(ngi.owner_ip_port_);
        reply_msg->set_role(ngi.owner_role_);
        reply_msg->set_status(ngi.owner_status_);
        if (SUCCESS == ret)
        {
          reply_msg->set_lease_id(lease_id);
          int32_t renew_lease_interval = SYSPARAM_NAMESERVER.heart_interval_ / 2;
          if (renew_lease_interval <= 0)
            renew_lease_interval = 1;
          reply_msg->set_lease_expired_time(SYSPARAM_NAMESERVER.heart_interval_);
          reply_msg->set_renew_lease_interval_time(renew_lease_interval);
          if (login)
          {
            manager_.get_oplog_sync_mgr().get_file_queue_thread()->update_queue_information_header();
          }
        }
      }
      ret = msg->reply(reply_msg);
    }
  }
  return ret;
}

void MetaServerHeartManager::CheckThreadHelper::run()
{
  try
  {
    manager_.check_();
  }
  catch(std::exception& e)
  {
    //LOG(ERROR, "catch exception: %s", e.what());
  }
  catch(...)
  {
    //LOG(ERROR, "%s", "catch exception, unknow message");
  }
}

void MetaServerHeartManager::check_()
{
  time_t now = 0;
  NsKeepAliveType keepalive_type_ = NS_KEEPALIVE_TYPE_LOGIN;
  int32_t sleep_time = SYSPARAM_NAMESERVER.heart_interval_ / 2;
  NsRuntimeGlobalInformation& ngi = GFactory::get_runtime_info();
  while (!ngi.is_destroyed())
  {
    now = Func::get_monotonic_time();
    ns_role_establish_(ngi, now);

    if (ngi.is_master())
    {
      ns_check_lease_expired_(ngi, now);
    }
    else
    {
      keepalive_(sleep_time, keepalive_type_, ngi);
      if (!ngi.has_valid_lease(now))
        keepalive_type_ = NS_KEEPALIVE_TYPE_LOGIN;
      else
        keepalive_type_ = NS_KEEPALIVE_TYPE_RENEW;
    }

    if (sleep_time <= 0)
    {
      sleep_time =  SYSPARAM_NAMESERVER.heart_interval_ / 2;
      sleep_time = std::max(sleep_time, 1);
    }
    Func::sleep(sleep_time, ngi.destroy_flag_);
  }
  keepalive_type_ = NS_KEEPALIVE_TYPE_LOGOUT;
  keepalive_(sleep_time, keepalive_type_, ngi);
}

bool MetaServerHeartManager::check_vip_(const NsRuntimeGlobalInformation& ngi) const
{
  return Func::is_local_addr(ngi.vip_);
}

int MetaServerHeartManager::ns_role_establish_(NsRuntimeGlobalInformation& ngi, const time_t now)
{
  if (check_vip_(ngi))//vip is local ip
  {
    if (!ngi.is_master())//slave, switch
      switch_role_salve_to_master_(ngi, now);
  }
  else
  {
    if (ngi.is_master())
      switch_role_master_to_slave_(ngi, now);
  }
  return SUCCESS;
}

void MetaServerHeartManager::switch_role_master_to_slave_(NsRuntimeGlobalInformation& ngi, const time_t now)
{
  if (ngi.is_master())
  {
    manager_.switch_role(now);
    //LOG(INFO, "metaserver switch, old role: master, current role: salve");
  }
}

void MetaServerHeartManager::switch_role_salve_to_master_(NsRuntimeGlobalInformation& ngi, const time_t now)
{
  if (!ngi.is_master())//slave, switch
  {
    int32_t ret = establish_peer_role_(ngi);
    if (EXIT_ROLE_ERROR != ret)
    {
      manager_.switch_role(now);
      //LOG(INFO, "metaserver switch, old role: slave, current role: master");
    }
  }
}

int MetaServerHeartManager::ns_check_lease_expired_(NsRuntimeGlobalInformation& ngi, const time_t now)
{
  if (!ngi.has_valid_lease(now))
  {
    ngi.logout();
  }
  return SUCCESS;
}

int MetaServerHeartManager::establish_peer_role_(NsRuntimeGlobalInformation& ngi)
{
  MasterAndSlaveHeartMessage msg;
  msg.set_ip_port(ngi.owner_ip_port_);
  msg.set_role(ngi.owner_role_);
  msg.set_status(ngi.owner_status_);
  msg.set_lease_id(ngi.lease_id_);
  msg.set_flags(HEART_GET_PEER_ROLE_FLAG_YES);
  int32_t ret = ERROR;
  const int32_t TIMEOUT_MS = 500;
  const int32_t MAX_RETRY_COUNT = 2;
  NewClient* client = NULL;
  Packet* response = NULL;
  for (int32_t i = 0; i < MAX_RETRY_COUNT && SUCCESS != ret; ++i)
  {
    client = NewClientManager::get_instance().create_client();
    ret = send_msg_to_server(ngi.peer_ip_port_, client, &msg, response, TIMEOUT_MS);
    if (SUCCESS == ret)
    {
      ret = response->getPCode() == MASTER_AND_SLAVE_HEART_RESPONSE_MESSAGE ? SUCCESS : EXIT_UNKNOWN_MSGTYPE;
      if (SUCCESS == ret)
      {
        MasterAndSlaveHeartResponseMessage* result = dynamic_cast<MasterAndSlaveHeartResponseMessage*>(response);
        bool conflict = result->get_role() == NS_ROLE_MASTER;//我将要变成master, 所以对方如果是master，那就有问题
        if (conflict)
        {
          ret = EXIT_ROLE_ERROR;
          //ngi.dump(//LOG_LEVEL_ERROR, "metaserver role coflict, own role: master, other role: master, must be exit");
          MetaServer* service = dynamic_cast<MetaServer*>(BaseMain::instance());
          if (NULL != service)
          {
            service->stop();
          }
        }
      }
    }
    NewClientManager::get_instance().destroy_client(client);
  }
  return ret;

}
int MetaServerHeartManager::keepalive_(int32_t& sleep_time, NsKeepAliveType& type, NsRuntimeGlobalInformation& ngi)
{
  MasterAndSlaveHeartMessage msg;
  msg.set_ip_port(ngi.owner_ip_port_);
  msg.set_role(ngi.owner_role_);
  msg.set_status(ngi.owner_status_);
  msg.set_lease_id(ngi.lease_id_);
  msg.set_type(type);
  int32_t interval = SYSPARAM_NAMESERVER.heart_interval_ / 2;
  interval = std::max(interval, 1);

  int32_t ret = ERROR;
  int32_t MAX_RETRY_COUNT = 2;
  int32_t MAX_TIMEOUT_TIME_MS = 500;
  NewClient* client = NULL;
  Packet* response = NULL;
  for (int32_t i = 0; i < MAX_RETRY_COUNT && SUCCESS != ret; ++i)
  {
    client = NewClientManager::get_instance().create_client();
    ret = send_msg_to_server(ngi.peer_ip_port_, client, &msg, response, MAX_TIMEOUT_TIME_MS);
    if (SUCCESS == ret)
    {
      ret = response->getPCode() == MASTER_AND_SLAVE_HEART_RESPONSE_MESSAGE ? SUCCESS : EXIT_UNKNOWN_MSGTYPE;
      if (SUCCESS == ret)
      {
        MasterAndSlaveHeartResponseMessage* result = dynamic_cast<MasterAndSlaveHeartResponseMessage*>(response);
        if (INVALID_LEASE_ID == result->get_lease_id())
        {
          ngi.logout();
        }
        else
        {
          ngi.renew(result->get_lease_id(), result->get_lease_expired_time(), Func::get_monotonic_time());
          sleep_time = result->get_renew_lease_interval_time();
          ngi.update_peer_info(result->get_ip_port(), result->get_role(), result->get_status());
        }
      }
    }
    NewClientManager::get_instance().destroy_client(client);
    //ngi.dump(//LOG_LEVEL_DEBUG);
  }
  return ret;
}

int MetaServerHeartManager::keepalive_in_heartbeat_(BasePacket* message)
{
  int32_t ret = NULL != message ? SUCCESS : EXIT_PARAMETER_ERROR;
  if (SUCCESS == ret)
  {
    ret = message->getPCode() == HEARTBEAT_AND_NS_HEART_MESSAGE ? SUCCESS : EXIT_UNKNOWN_MSGTYPE;
    if (SUCCESS == ret)
    {
      NsRuntimeGlobalInformation& ngi = GFactory::get_runtime_info();
      HeartBeatAndNSHeartMessage* hbam = dynamic_cast<HeartBeatAndNSHeartMessage*> (message);
      int32_t ns_switch_flag = hbam->get_ns_switch_flag();
      //LOG(DEBUG, "ns_switch_flag: %s, status: %d",
      //    hbam->get_ns_switch_flag() == NS_SWITCH_FLAG_NO ? "no" : "yes", hbam->get_ns_status());
      HeartBeatAndNSHeartMessage* mashrm = new HeartBeatAndNSHeartMessage();
      mashrm->set_ns_switch_flag_and_status(0 /*no use*/ , ngi.owner_status_);
      message->reply(mashrm);
      if (ns_switch_flag == NS_SWITCH_FLAG_YES)
      {
        //LOG(WARN, "ns_switch_flag: %s, status: %d", hbam->get_ns_switch_flag() == NS_SWITCH_FLAG_NO ? "no"
        //    : "yes", hbam->get_ns_status());
        if (check_vip_(ngi))
        {
          if (!ngi.is_master())
            switch_role_salve_to_master_(ngi, Func::get_monotonic_time());
        }
      }
    }
  }
  return ret;
}

}
}
}
