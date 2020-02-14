#include "dfs/util/dfs.h"
#include <libgen.h>
#include "base/common/ErrorMsg.h"
#include "config_item.h"
#include "base_service.h"
#include "base/fs/DirectoryOp.h"
#include "local_packet.h"

namespace neptune {
namespace dfs {

int BaseService::golbal_async_callback_func(NewClient* client, void* args)
{
  BaseService* service = dynamic_cast<BaseService*>(BaseService::instance());
  int32_t iret = NULL != service ? SUCCESS : ERROR;
  if (SUCCESS == iret)
  {
    iret = service->async_callback(client, args);
  }
  return iret;
}

BaseService::BaseService():
  packet_factory_(NULL),
  streamer_(NULL),
  timer_(0),
  work_queue_size_(10240)
{

}

BaseService::~BaseService()
{

}

bool BaseService::destroy()
{
  if (NULL != transport_)
    transport_->stop();
  main_workers_.stop();
  destroy_service();
  NewClientManager::get_instance().destroy();
  if (0 != timer_)
  {
    timer_->destroy();
  }

  if (NULL != transport_)
    transport_->wait();
  main_workers_.wait();

  destroy_packet_factory(packet_factory_);
  destroy_packet_streamer(streamer_);
  gDelete(transport_);
  return true;
}

int BaseService::async_callback(NewClient* client, void*)
{
  assert(NULL != client);
  LocalPacket* packet = dynamic_cast<LocalPacket*>(packet_factory_->createPacket(LOCAL_PACKET));
  assert(NULL != packet);
  packet->set_new_client(client);
  bool bret = main_workers_.push(packet, 0/*no limit*/, false/*no block*/);
  assert(true == bret);
  return SUCCESS;
}

bool BaseService::push(BasePacket* packet, bool block)
{
  return main_workers_.push(packet, work_queue_size_, block);
}

IPacketHandler::HPRetCode BaseService::handlePacket(Connection *connection, Packet *packet)
{
  //LOG(DEBUG, "packet code : %d", packet->getPCode());
  bool bret = NULL != connection && NULL != packet;
  if (bret)
  {
    if (!packet->isRegularPacket())
    {
      bret = false;
      //LOG(WARN, "control packet, pcode: %d", dynamic_cast<ControlPacket*>(packet)->getCommand());
    }
    if (bret)
    {
      BasePacket* bpacket = dynamic_cast<BasePacket*>(packet);
      bpacket->set_connection(connection);
      bpacket->setExpireTime(MAX_RESPONSE_TIME);
      bpacket->set_direction(static_cast<DirectionStatus>(bpacket->get_direction()|DIRECTION_RECEIVE));

      if (bpacket->is_enable_dump())
      {
        bpacket->dump();
      }
      if (!main_workers_.push(bpacket, work_queue_size_, false))
      {
        //bpacket->reply_error_packet(//LOG_LEVEL(ERROR),STATUS_MESSAGE_ERROR, "%s, task message beyond max queue size, discard", get_ip_addr());
        bpacket->free();
      }
    }
  }
  return IPacketHandler::FREE_CHANNEL;
}

/** Note if return true, PacketQueueThread will delete this packet*/
bool BaseService::handlePacketQueue(Packet *packet, void *)
{
  bool bret = true;
  if (NULL == packet)
  {
    bret = false;
    //LOG(ERROR, "%s", "invalid packet, packet is null");
  }
  else
  {
    if (LOCAL_PACKET == packet->getPCode())
    {
      LocalPacket* local_packet = dynamic_cast<LocalPacket*>(packet);
      int32_t iret = local_packet->execute();
      if (SUCCESS != iret)
      {
        //LOG(ERROR, " LocalPacket execute error, iret: %d", iret);
      }
    }
  }
  return bret;
}

int32_t BaseService::get_port() const
{
  int32_t port = -1;
  port = NEP_CONFIG.getInt(CONF_SN_PUBLIC, CONF_PORT, -1);
  if (port < 1024 || port > 65535)
  {
    port = -1;
  }
  return port;
}

const char* const BaseService::get_dev() const
{
  return NEP_CONFIG.getString(CONF_SN_PUBLIC, CONF_DEV_NAME, NULL);
}

int32_t BaseService::get_work_thread_count() const
{
  return NEP_CONFIG.getInt(CONF_SN_PUBLIC, CONF_THREAD_COUNT, 8);
}

int32_t BaseService::get_work_queue_size() const
{
  return work_queue_size_;
}

const char* BaseService::get_ip_addr() const
{
  return NEP_CONFIG.getString(CONF_SN_PUBLIC, CONF_IP_ADDR, NULL);
}

int BaseService::run(int argc , char*argv[])
{
  //check ip, port, starrt tbnet
  int32_t iret = initialize_network(argv[0]);
  if (SUCCESS != iret)
  {
    LOG(ERROR, "%s initialize network failed, must be exit", argv[0]);
  }

  //start workthread
  if (SUCCESS == iret)
  {
    int32_t thread_count = get_work_thread_count();
    main_workers_.setThreadParameter(thread_count, this, NULL);
    main_workers_.start();

    work_queue_size_ = NEP_CONFIG.getInt(CONF_SN_PUBLIC, CONF_TASK_MAX_QUEUE_SIZE, 10240);
    work_queue_size_ = std::max(work_queue_size_, 10240);
    work_queue_size_ = std::min(work_queue_size_, 40960);
    timer_ = new Timer();
  }

  if (SUCCESS == iret)
  {
    iret = initialize(argc, argv);
    if (SUCCESS != iret)
    {
      LOG(ERROR, "%s initialize user data failed, must be exit", argv[0]);
    }
  }
  return iret;
}

int BaseService::initialize_network(const char* app_name)
{
  int32_t iret = SUCCESS;
  const char* ip_addr = get_ip_addr();
  if (NULL == ip_addr)//get ip addr
  {
    iret =  EXIT_CONFIG_ERROR;
    LOG(ERROR, "%s not set ip_addr", app_name);
  }

  int32_t port = 0;
  if (SUCCESS == iret)
  {
    port  = get_listen_port();//check port
    if (port <= 0)
    {
      iret = EXIT_CONFIG_ERROR;
      LOG(ERROR, "%s not set port: %d or port: %d is invalid", app_name, port, port);
    }
  }

  //start transport
  if (SUCCESS == iret)
  {
    char spec[32];
    sprintf(spec, "tcp::%d", port);

    packet_factory_ = create_packet_factory();
    if (NULL == packet_factory_)
    {
      iret = EXIT_GENERAL_ERROR;
      LOG(ERROR, "%s create packet factory fail", app_name);
    }
    if (SUCCESS == iret)
    {
      streamer_ = dynamic_cast<BasePacketStreamer*>(create_packet_streamer());
      if (NULL == streamer_)
      {
        LOG(ERROR, "%s create packet streamer fail", app_name);
        iret = EXIT_GENERAL_ERROR;
      }
      else
      {
        transport_ = new Transport();
        streamer_->set_packet_factory(packet_factory_);
        IOComponent* com = transport_->listen(spec, streamer_, this);
        if (NULL == com)
        {
          LOG(ERROR, "%s listen port: %d fail", app_name, port);
          iret = EXIT_NETWORK_ERROR;
        }
        else
        {
          transport_->start();
        }
      }
    }
  }

  // start client manager
  if (SUCCESS == iret)
  {
    iret = NewClientManager::get_instance().initialize(packet_factory_, streamer_,
            transport_, &BaseService::golbal_async_callback_func, this);
    if (SUCCESS != iret)
    {
      LOG(ERROR, "%s start client manager fail", app_name);
      iret = EXIT_NETWORK_ERROR;
    }
  }
  return iret;
}

} //namespace dfs
} //namespace neptune
