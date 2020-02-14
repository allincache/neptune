#include "base/common/Memory.h"
#include "requester.h"
#include "../message/block_info_message.h"
#include "dfs/util/new_client.h"
#include "dfs/util/client_manager.h"
#include "dfs/util/status_message.h"

namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;
using namespace neptune::dfs;

int Requester::init(const uint64_t dataserver_id, const uint64_t ns_ip_port, DataManagement* data_management)
{
  dataserver_id_ = dataserver_id;
  ns_ip_port_ = ns_ip_port;
  data_management_ = data_management;
  return SUCCESS;
}

int Requester::req_update_block_info(const uint32_t block_id, const UpdateBlockType repair)
{
  UpdateBlockType tmp_repair = repair; 
  BlockInfo* blk = NULL;
  if (UPDATE_BLOCK_MISSING != tmp_repair)
  {
    int32_t visit_count = 0;
    int ret = data_management_->get_block_info(block_id, blk, visit_count);
    if (EXIT_NO_LOGICBLOCK_ERROR == ret)
    {
      tmp_repair = UPDATE_BLOCK_MISSING;
    }
    else
    {
      if (NULL == blk)
      {
        //LOG(ERROR, "blockid: %u can not find block info.", block_id);
        tmp_repair = UPDATE_BLOCK_REPAIR;
      }
      else
      {
        //LOG(
        //    INFO,
        //    "req update block info, blockid: %u, version: %d, file count: %d, size: %d, delfile count: %d, del_size: %d, seqno: %d\n",
        //    blk->block_id_, blk->version_, blk->file_count_, blk->size_, blk->del_file_count_, blk->del_size_, blk->seq_no_);
      }
    }
  }

  int ret = ERROR;
  UpdateBlockInfoMessage ub_msg;
  ub_msg.set_block_id(block_id);
  ub_msg.set_block(blk);
  ub_msg.set_server_id(dataserver_id_);
  ub_msg.set_repair(tmp_repair);
  NewClient* client = NewClientManager::get_instance().create_client();
  Packet* return_msg = NULL;
  ret = send_msg_to_server(ns_ip_port_, client, &ub_msg, return_msg);
  if (SUCCESS != ret)
  {
    NewClientManager::get_instance().destroy_client(client);
    return ret;
  }
  int need_expire = 0;
  if (STATUS_MESSAGE == return_msg->getPCode())
  {
    StatusMessage* sm = dynamic_cast<StatusMessage*>(return_msg);
    if (STATUS_MESSAGE_OK == sm->get_status())
    {
      ret = SUCCESS;
    }
    else if (STATUS_MESSAGE_REMOVE == sm->get_status())
    {
      need_expire = 1;
      ret = SUCCESS;
    }
    else
    {
      //LOG(ERROR, "req update block info: %s, id: %u, tmp_repair: %d\n", sm->get_error(), block_id, tmp_repair);
    }
  }
  else
  {
    //LOG(ERROR,"unknow packet pcode: %d", return_msg->getPCode());
  }
  NewClientManager::get_instance().destroy_client(client);

  if (need_expire)
  {
    data_management_->del_single_block(block_id);
  }

  return ret;
}

int Requester::req_block_write_complete(const uint32_t block_id,
    const int32_t lease_id, const int32_t success, const UnlinkFlag unlink_flag)
{
  //LOG(DEBUG, "req block write complete begin id: %u, lease_id: %u\n", block_id, lease_id);

  BlockInfo* blk = NULL;
  int visit_count = 0;
  int ret = data_management_->get_block_info(block_id, blk, visit_count);
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "req block write complete: can not find block, id: %u, ret: %d\n", block_id, ret);
    return ERROR;
  }

  BlockInfo tmpblk;
  memcpy(&tmpblk, blk, sizeof(BlockInfo));
  ret = data_management_->get_block_curr_size(block_id, tmpblk.size_);
  if (SUCCESS != ret)
  {
    //LOG(ERROR, "req block write complete: can not find block, id: %u, ret: %d\n", block_id, ret);
    return ERROR;
  }

  BlockWriteCompleteMessage bwc_msg;
  bwc_msg.set_block(&tmpblk);
  bwc_msg.set_server_id(dataserver_id_);
  bwc_msg.set_lease_id(lease_id);
  WriteCompleteStatus wc_status = WRITE_COMPLETE_STATUS_YES;
  if (SUCCESS != success)
  {
    wc_status = WRITE_COMPLETE_STATUS_NO;
  }
  bwc_msg.set_success(wc_status);
  bwc_msg.set_unlink_flag(unlink_flag);

  NewClient* client = NewClientManager::get_instance().create_client();
  Packet* return_msg = NULL;
  ret = send_msg_to_server(ns_ip_port_, client, &bwc_msg, return_msg);


  if (SUCCESS != ret)
  {
    NewClientManager::get_instance().destroy_client(client);
    return ret;
  }

  if (STATUS_MESSAGE == return_msg->getPCode())
  {
    StatusMessage* sm = dynamic_cast<StatusMessage*>(return_msg);
    if (STATUS_MESSAGE_OK == sm->get_status())
    {
      ret = SUCCESS;
    }
    else
    {
      ret = ERROR;
      //LOG(ERROR, "req block write complete, nsip: %s, error desc: %s, id: %u\n",
      //    CNetUtil::addrToString(ns_ip_port_).c_str(), sm->get_error(), block_id);
    }
  }
  else
  {
    //LOG(ERROR, "req block write complete, blockid: %u, msg type: %d error.\n", block_id,
    //    return_msg->getPCode());
    ret = ERROR;
  }
  NewClientManager::get_instance().destroy_client(client);
  return ret;
}
  
}
}
}
