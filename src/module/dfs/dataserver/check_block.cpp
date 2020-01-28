#include "base/time/Time.h"
#include "base/common/Memory.h"
#include "dfs/message/replicate_block_message.h"
#include "dfs/message/write_data_message.h"
#include "dfs/util/client_manager.h"
#include "dfs/util/new_client.h"
#include "dfs/util/status_message.h"
#include "blockfile_manager.h"
#include "check_block.h"


namespace neptune {
namespace dfs {
namespace dataserver {

using namespace neptune::dfs;
using namespace neptune::base;

void CheckBlock::add_check_task(const uint32_t block_id)
{
  changed_block_mutex_.lock();
  ChangedBlockMapIter iter = changed_block_map_.find(block_id);
  if (iter == changed_block_map_.end())
  {
    uint32_t mtime = time(NULL);
    changed_block_map_.insert(std::make_pair(block_id, mtime));
    //LOG(DEBUG, "add check task. block id: %u, modify time: %u", block_id, mtime);
  }
  else
  {
    iter->second = time(NULL);
    //LOG(DEBUG, "update check task. block id: %u, modify time: %u",
    //    iter->first, iter->second);
  }
  changed_block_mutex_.unlock();
}

void CheckBlock::remove_check_task(const uint32_t block_id)
{
  changed_block_mutex_.lock();
  ChangedBlockMapIter iter = changed_block_map_.find(block_id);
  if (iter != changed_block_map_.end())
  {
    changed_block_map_.erase(iter);
  }
  changed_block_mutex_.unlock();
}

int CheckBlock::check_all_blocks(CheckBlockInfoVec& check_result,
    const int32_t check_flag, const uint32_t check_time, const uint32_t last_check_time)
{
  LOG(DEBUG, "check all blocks: check_flag: %d", check_flag);
  TIMER_START();

  // get the list need to check, then free lock
  UNUSED(check_flag);
  VUINT should_check_blocks;
  changed_block_mutex_.lock();
  ChangedBlockMapIter iter = changed_block_map_.begin();
  for ( ; iter != changed_block_map_.end(); iter++)
  {
    // check if modify time between check range
    if (iter->second < check_time &&
        iter->second >= last_check_time)
    {
      should_check_blocks.push_back(iter->first);
    }
  }
  changed_block_mutex_.unlock();

  // do real check, call check_one_block
  int ret = 0;
  CheckBlockInfo result;
  VUINT::iterator it = should_check_blocks.begin();
  for ( ; it != should_check_blocks.end(); it++)
  {
      ret = check_one_block(*it, result);
      if (SUCCESS == ret)
      {
        check_result.push_back(result);
      }
  }
  TIMER_END();
  //LOG(INFO, "check all blocks. count: %zd, cost: %"PRI64_PREFIX"d\n",
  //    should_check_blocks.size(), TIMER_DURATION());

  return SUCCESS;
}

int CheckBlock::check_one_block(const uint32_t block_id, CheckBlockInfo& result)
{
  //LOG(DEBUG, "check one, block_id: %u", block_id);
  int ret = SUCCESS;
  LogicBlock* logic_block = BlockFileManager::get_instance()->get_logic_block(block_id);
  if (NULL == logic_block)  // already deleted block, remove it
  {
    //LOG(WARN, "blockid: %u is not exist.", block_id);
    remove_check_task(block_id);
    ret = EXIT_NO_LOGICBLOCK_ERROR;
  }
  else
  {
    logic_block->rlock();   // lock block
    BlockInfo bi;
    ret = logic_block->get_block_info(&bi);
    logic_block->unlock();

    if (SUCCESS == ret)
    {
      result.block_id_ = bi.block_id_;    // block id
      result.version_ = bi.version_;     // version
      result.file_count_ = bi.file_count_ - bi.del_file_count_; // file count
      result.total_size_ = bi.size_ - bi.del_size_;   // file size
    }

    //LOG(DEBUG, "blockid: %u, file count: %u, total size: %u",
    //      result.block_id_, result.file_count_, result.total_size_);
  }
  return ret;
}

int CheckBlock::repair_block_info(const uint32_t block_id)
{
  //LOG(DEBUG, "repair block info %u", block_id);
  int ret = SUCCESS;
  LogicBlock* logic_block = BlockFileManager::get_instance()->get_logic_block(block_id);
  if (NULL == logic_block)
  {
  //  LOG(WARN, "block %u not found\n", block_id);
  }
  else
  {
    BlockInfo bi;
    BlockInfo bi_old;
    BlockInfo bi_new;
    RawMetaVec raw_metas;
    RawMetaVecIter meta_it;

    // rlock block
    logic_block->rlock();
    ret = logic_block->get_block_info(&bi);
    if (SUCCESS == ret)
    {
      bi_old = bi;
      logic_block->get_meta_infos(raw_metas);
    }
    logic_block->unlock();

    // repair, lock free
    FileInfo fi;
    if (SUCCESS == ret)
    {
      bi.file_count_ =  0;
      bi.size_ = 0;
      bi.del_file_count_ = 0;
      bi.del_size_ = 0;
      meta_it = raw_metas.begin();
      for ( ; meta_it != raw_metas.end(); meta_it++)
      {
        int32_t offset = meta_it->get_offset();
        int32_t size = meta_it->get_size();
        int32_t finfo_size = sizeof(FileInfo);
        if (SUCCESS == logic_block->read_raw_data((char*)&fi, finfo_size, offset))
        {
          if (fi.flag_ & FI_DELETED)
          {
            bi.del_file_count_++;
            bi.del_size_ += size;
          }
          bi.file_count_++;
          bi.size_ += size;
        }
      }
    }

    if (SUCCESS == ret)
    {
      // update, block may be deleted already
      logic_block = BlockFileManager::get_instance()->get_logic_block(block_id);
      if (NULL == logic_block)
      {
      //  LOG(WARN, "block %u not found\n", block_id);
      }
      else
      {
        // wlock block
        logic_block->wlock();
        ret = logic_block->get_block_info(&bi_new);
        if (SUCCESS == ret && bi_new == bi_old && !(bi_old == bi))
        {

        //  LOG(DEBUG, "will update block info");
          ret = logic_block->copy_block_info(&bi);
        }
        logic_block->unlock();
      }
    }

    // result
    if (SUCCESS == ret)
    {
    //  LOG(INFO, "repair block info %u succeed", block_id);
    }
    else
    {
    //  LOG(WARN, "repair block info %u fail, ret: %d", block_id, ret);
    }
  }
  return ret;
}

} //namespace dataserver
} //namespace dfs
} //namespace neptune
