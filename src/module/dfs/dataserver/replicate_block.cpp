#include "base/time/Time.h"
#include "base/common/Memory.h"
#include "dfs/message/replicate_block_message.h"
#include "dfs/message/write_data_message.h"
#include "dfs/util/client_manager.h"
#include "dfs/util/new_client.h"
#include "dfs/util/status_message.h"
#include "replicate_block.h"


namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::base;
using namespace neptune::dfs;

ReplicateBlock::ReplicateBlock(const uint64_t ns_ip)
{
  init();
  ns_ip_ = ns_ip;
}

ReplicateBlock::~ReplicateBlock()
{
}

void ReplicateBlock::stop()
{
  stop_ = 1;
  clear_cloned_block_map();
  repl_block_monitor_.lock();
  repl_block_monitor_.notifyAll();
  repl_block_monitor_.unlock();
}

void ReplicateBlock::init()
{
  expire_cloned_interval_ = SYSPARAM_DATASERVER.expire_cloned_block_time_;
  last_expire_cloned_block_time_ = 0;
  stop_ = 0;
}

int ReplicateBlock::run_replicate_block()
{
  Time timeout = Time::seconds(1);
  while (!stop_)
  {
    repl_block_monitor_.lock();
    while (!stop_)
    {
      repl_block_monitor_.timedWait(timeout);
      // wait the first one
      if (!repl_block_queue_.empty())
      {
        break;
      }
    }

    if (stop_)
    {
      repl_block_monitor_.unlock();
      break;
    }

    ReplBlockExt b = repl_block_queue_.front();
    //LOG(INFO, "repl block blockid: %d", b.info_.block_id_);
    repl_block_queue_.pop_front();
    replicating_block_map_[b.info_.block_id_] = b;
    repl_block_monitor_.unlock();

    //replicate
    int64_t start_time = Func::curr_time();
    int ret = replicate_block_to_server(b);
    int result = send_repl_block_complete_info(ret, b);
    int64_t end_time = Func::curr_time();
    //LOG(INFO, "replicate %s blockid: %u, %s=>%s, cost time: %"PRI64_PREFIX"d (ms), commit: %s", (ret ? "fail" : "success"),
    //    b.info_.block_id_, CNetUtil::addrToString(b.info_.source_id_).c_str(), CNetUtil::addrToString(
    //        b.info_.destination_id_).c_str(), (end_time - start_time) / 1000, SUCCESS == result ? "successful" : "failed");

    repl_block_monitor_.lock();
    replicating_block_map_.erase(b.info_.block_id_);
    repl_block_monitor_.unlock();
  }

  repl_block_monitor_.lock();
  while (!repl_block_queue_.empty())
  {
    ReplBlockExt b = repl_block_queue_.front();
    repl_block_queue_.pop_front();
  }

  repl_block_monitor_.unlock();
  return SUCCESS;
}

int ReplicateBlock::send_repl_block_complete_info(const int status, const ReplBlockExt& b)
{
  ReplicateBlockMessage req_rb_msg;
  int ret = ERROR;

  req_rb_msg.set_seqno(b.seqno_);
  req_rb_msg.set_repl_block(&b.info_);
  if (SUCCESS == status)
  {
    req_rb_msg.set_command(PLAN_STATUS_END);
  }
  else
  {
    req_rb_msg.set_command(PLAN_STATUS_FAILURE);
  }

  bool need_remove = false;
  NewClient* client = NewClientManager::get_instance().create_client();
  if (NULL != client)
  {
    Packet* rsp_msg = NULL;
    if (SUCCESS == send_msg_to_server(ns_ip_, client, &req_rb_msg, rsp_msg))
    {
      if (STATUS_MESSAGE != rsp_msg->getPCode())
      {
        //LOG(ERROR, "unknow packet pcode: %d", rsp_msg->getPCode());
      }
      else
      {
        StatusMessage* sm = dynamic_cast<StatusMessage*> (rsp_msg);
        if (b.info_.is_move_ == REPLICATE_BLOCK_MOVE_FLAG_YES && STATUS_MESSAGE_REMOVE == sm->get_status())
        {
          need_remove = true;
          ret = SUCCESS;
        }
        else if (STATUS_MESSAGE_OK == sm->get_status())
        {
          ret = SUCCESS;
        }
        else
        {
          //LOG(ERROR, "send repl block complete info: %s\n", sm->get_error());
        }
      }
    }
    else
    {
      //LOG(ERROR, "send repl block complete info to ns error, ret: %d", ret);
    }
    NewClientManager::get_instance().destroy_client(client);
  }
  else
  {
    //LOG(ERROR, "create client error");
  }

  if (need_remove)
  {
    int rm_ret = BlockFileManager::get_instance()->del_block(b.info_.block_id_);
    //LOG(INFO, "send repl block complete info: del blockid: %u, result: %d\n", b.info_.block_id_, rm_ret);
  }
  return ret;
}

// replicate one block to other ds
int ReplicateBlock::replicate_block_to_server(const ReplBlockExt& b)
{
  uint64_t ds_ip = b.info_.destination_id_;
  uint32_t block_id = b.info_.block_id_;

  //LOG(INFO, "replicating now, blockid: %u, %s = >%s\n", block_id, CNetUtil::addrToString(
  //    b.info_.source_id_).c_str(), CNetUtil::addrToString(ds_ip).c_str());

  LogicBlock* logic_block = BlockFileManager::get_instance()->get_logic_block(block_id);
  if (NULL == logic_block)
  {
    //LOG(ERROR, "block is not exist, blockid: %u, %s=>%s\n", b.info_.block_id_, CNetUtil::addrToString(
    //    b.info_.source_id_).c_str(), CNetUtil::addrToString(ds_ip).c_str());
    return ERROR;
  }

  // send to port + 1
  //ds_ip = Func::addr_inc_port(ds_ip, 1);

  //replicate block file
  int32_t len = 0, offset = 0;
  int ret = SUCCESS;
  char tmp_data_buf[MAX_READ_SIZE];

  //this block will not be write or update now, locked by ns
  int32_t total_len = logic_block->get_data_file_size();
  while (offset < total_len || 0 == total_len)
  {
    int32_t read_len = MAX_READ_SIZE;
    ret = logic_block->read_raw_data(tmp_data_buf, read_len, offset);
    if (SUCCESS != ret)
    {
      //LOG(ERROR, "read raw data fail, ip: %s, blockid: %u, offset: %d, reading len: %d, ret: %d",
      //    CNetUtil::addrToString(ds_ip).c_str(), block_id, offset, read_len, ret);
      break;
    }
    len = read_len;

    //LOG(DEBUG, "replicate read raw data blockid: %u, offset: %d, read len: %d, total len: %d\n", block_id,
    //    offset, len, total_len);

    WriteRawDataMessage req_wrd_msg;
    req_wrd_msg.set_block_id(block_id);
    req_wrd_msg.set_offset(offset);
    req_wrd_msg.set_length(len);
    req_wrd_msg.set_data(tmp_data_buf);

    //new block		
    if (0 == offset)
    {
      req_wrd_msg.set_new_block(1);
    }
    NewClient* client = NewClientManager::get_instance().create_client();
    if (NULL != client)
    {
      Packet* rsp_msg = NULL;
      if (SUCCESS == send_msg_to_server(ds_ip, client, &req_wrd_msg, rsp_msg))
      {
        if (rsp_msg->getPCode() == STATUS_MESSAGE)
        {
          StatusMessage* sm = dynamic_cast<StatusMessage*> (rsp_msg);
          if (STATUS_MESSAGE_OK != sm->get_status())
          {
            //LOG(ERROR, "write raw data to %s fail, blockid: %u, offset:%d, reading len: %d",
            //    CNetUtil::addrToString(ds_ip).c_str(), block_id, offset, len);
            ret = ERROR;
          }
        }
      }
      else
      {
        //LOG(ERROR, "write raw data to %s fail, blockid: %u, offset: %u, reading len: %d",
        //    CNetUtil::addrToString(ds_ip).c_str(), block_id, offset, len);
        ret = ERROR;
      }
      NewClientManager::get_instance().destroy_client(client);
    }
    else
    {
      //LOG(ERROR, "create client error");
    }

    if (SUCCESS != ret)
    {
      break;
    }

    offset += len;
    if (len != MAX_READ_SIZE)
    {
      break;
    }
  }

  if (SUCCESS == ret)
  {
    //use read index file direct maybe faster
    RawMetaVec raw_meta_vec;
    ret = logic_block->get_meta_infos(raw_meta_vec);
    if (SUCCESS != ret)
    {
      //LOG(ERROR, "replicate get meta info fail, server: %s, blockid: %u, ret: %d",
      //    CNetUtil::addrToString(ds_ip).c_str(), block_id, ret);
      return ERROR;
    }

    WriteInfoBatchMessage req_wib_msg;
    req_wib_msg.set_block_id(block_id);
    req_wib_msg.set_offset(0);
    req_wib_msg.set_length(raw_meta_vec.size());
    req_wib_msg.set_raw_meta_list(&raw_meta_vec);
    req_wib_msg.set_block_info(logic_block->get_block_info());
    if (COPY_BETWEEN_CLUSTER == b.info_.server_count_)
    {
      req_wib_msg.set_cluster(COPY_BETWEEN_CLUSTER);
    }

    //LOG(DEBUG, "replicate get meta info. blockid: %u, meta info size: %zd, cluster flag: %d\n", block_id,
    //    raw_meta_vec.size(), req_wib_msg.get_cluster());

    ret = SUCCESS;
    NewClient* client = NewClientManager::get_instance().create_client();
    if (NULL != client)
    {
      Packet* rsp_msg = NULL;
      if (SUCCESS == send_msg_to_server(ds_ip, client, &req_wib_msg, rsp_msg))
      {
        if (STATUS_MESSAGE == rsp_msg->getPCode())
        {
          StatusMessage* sm = dynamic_cast<StatusMessage*> (rsp_msg);
          if (STATUS_MESSAGE_OK != sm->get_status())
          {
            //LOG(ERROR, "write meta info to %s fail, blockid: %u", CNetUtil::addrToString(ds_ip).c_str(),
            //    block_id);
            ret = ERROR;
          }
        }
        else
        {
          //LOG(ERROR, "unknow packet pcode :%d", rsp_msg->getPCode());
        }
      }
      else
      {
        //LOG(ERROR, "write meta info to %s fail, blockid: %u", CNetUtil::addrToString(ds_ip).c_str(),
        //    block_id);
        ret = ERROR;
      }
      NewClientManager::get_instance().destroy_client(client);
    }
    else
    {
      //LOG(ERROR, "create client error");
    }
  }
  return ret;
}

int ReplicateBlock::add_repl_task(ReplBlockExt& tmp_rep_blk)
{
  int repl_exist = 0;
  repl_block_monitor_.lock();
  if (replicating_block_map_.find(tmp_rep_blk.info_.block_id_) != replicating_block_map_.end())
  {
    repl_exist = 1;
  }
  else
  {
    for (uint32_t i = 0; i < repl_block_queue_.size(); ++i)
    {
      if (repl_block_queue_[i].info_.block_id_ == tmp_rep_blk.info_.block_id_)
      {
        repl_exist = 1;
        break;
      }
    }
  }

  //LOG(DEBUG, "add repl task. blockid: %u, is exist: %d\n", tmp_rep_blk.info_.block_id_, repl_exist);

  if (0 == repl_exist)
  {
    tmp_rep_blk.info_.start_time_ = time(NULL);
    repl_block_queue_.push_back(tmp_rep_blk);
  }
  repl_block_monitor_.unlock();
  repl_block_monitor_.lock();
  repl_block_monitor_.notify();
  repl_block_monitor_.unlock();

  return repl_exist;
}

int ReplicateBlock::add_cloned_block_map(const uint32_t block_id)
{
  ClonedBlock* cloned_block = new ClonedBlock();

  cloned_block->blockid_ = block_id;
  cloned_block->start_time_ = time(NULL);
  cloned_block_mutex_.lock();
  cloned_block_map_.insert(ClonedBlockMap::value_type(block_id, cloned_block));
  cloned_block_mutex_.unlock();

  return SUCCESS;
}

int ReplicateBlock::del_cloned_block_map(const uint32_t block_id)
{
  //LOG(DEBUG, "del cloned block map blockid :%u", block_id);
  cloned_block_mutex_.lock();
  ClonedBlockMapIter mit = cloned_block_map_.find(block_id);
  if (mit != cloned_block_map_.end())
  {
    gDelete(mit->second);
    cloned_block_map_.erase(mit);
  }
  cloned_block_mutex_.unlock();

  return SUCCESS;
}

int ReplicateBlock::clear_cloned_block_map()
{
  int ret = SUCCESS;
  cloned_block_mutex_.lock();
  for (ClonedBlockMapIter mit = cloned_block_map_.begin(); mit != cloned_block_map_.end(); ++mit)
  {
    ret = BlockFileManager::get_instance()->del_block(mit->first);
    if (SUCCESS != ret)
    {
      //LOG(ERROR, "in check thread: del blockid: %u error. ret: %d", mit->first, ret);
    }

    gDelete(mit->second);
  }
  cloned_block_map_.clear();
  cloned_block_mutex_.unlock();

  return SUCCESS;
}

int ReplicateBlock::expire_cloned_block_map()
{
  int ret = SUCCESS;
  int32_t current_time = time(NULL);
  int32_t now_time = current_time - expire_cloned_interval_;
  if (last_expire_cloned_block_time_ < now_time)
  {
    cloned_block_mutex_.lock();
    int old_cloned_block_size = cloned_block_map_.size();
    for (ClonedBlockMapIter mit = cloned_block_map_.begin(); mit != cloned_block_map_.end();)
    {
      if (now_time < 0)
        break;
      if (mit->second->start_time_ < now_time)
      {
        ret = BlockFileManager::get_instance()->del_block(mit->first);
        if (SUCCESS != ret)
        {
          //LOG(ERROR, "in check thread: del blockid: %u error. ret: %d", mit->first, ret);
        }

        gDelete(mit->second);
        cloned_block_map_.erase(mit++);
      }
      else
      {
        ++mit;
      }
    }

    int32_t new_cloned_block_size = cloned_block_map_.size();
    last_expire_cloned_block_time_ = current_time;
    cloned_block_mutex_.unlock();
    //LOG(INFO, "cloned block map: old: %d, new: %d", old_cloned_block_size, new_cloned_block_size);
  }
  return SUCCESS;
}

}
}
}
