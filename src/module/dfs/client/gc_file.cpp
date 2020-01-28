#include "base/common/Memory.h"
#include "dfs/util/dfs.h"
#include "base/fs/DirectoryOp.h"
#include "base/common/ErrorMsg.h"
#include "gc_file.h"

using namespace neptune::dfs;

const char* GC_FILE_PATH = "/tmp/DFSlocalkeyDIR/gc/";

GcFile::GcFile(const bool need_save_seg_infos)
  :need_save_seg_infos_(need_save_seg_infos), file_pos_(sizeof(SegmentHead))
{
}

GcFile::~GcFile()
{
  // no gc segment, remove file
  if (file_op_ != NULL)
  {
    if (0 == seg_head_.count_)
    {
      //LOG(DEBUG, "no gc info, remove file");
      file_op_->unlink_file();
    }
    else if (need_save_seg_infos_) // not load, save remaining segment infos
    {
      save_gc();
    }
  }
}

int GcFile::initialize(const char* name)
{
  int ret = SUCCESS;

  if (NULL == name)
  {
    //LOG(ERROR, "null gc file name");
    ret = ERROR;
  }
  else if (!DirectoryOp::create_full_path(GC_FILE_PATH, false, GC_FILE_PATH_MODE))
  {
    //LOG(ERROR, "create gc file path %s with mode %d fail, error: %s",
    //          GC_FILE_PATH, GC_FILE_PATH_MODE, strerror(errno));
    ret =ERROR;
  }
  else
  {
    char file_path[MAX_PATH_LENGTH];
    snprintf(file_path, MAX_PATH_LENGTH, "%s%s", GC_FILE_PATH, name);

    if (access(file_path, F_OK) != 0)
    {
      ret = init_file_op(file_path, O_RDWR|O_CREAT);
    }
    else if ((ret = init_file_op(file_path, O_RDWR|O_APPEND)) == SUCCESS)
    {
      load_head();
      //LOG(DEBUG, "load head count: %d, size: %"PRI64_PREFIX"d", seg_head_.count_, seg_head_.size_);
      //LOG(DEBUG, "initialize gc file success");
    }
  }
  return ret;
}

int GcFile::add_segment(const SegmentInfo& seg_info)
{
  int ret = SUCCESS;
  seg_info_.push_back(seg_info);
  seg_head_.count_++;
  seg_head_.size_ += seg_info.size_;

  //LOG(DEBUG, "add gc segment. blockid: %u, fileid: %"PRI64_PREFIX"u, offset: %"PRI64_PREFIX"d, size: %d, crc: %u",
  //          seg_info.block_id_, seg_info.file_id_, seg_info.offset_, seg_info.size_, seg_info.crc_);
  return ret;
}

int GcFile::validate(const int64_t)
{
  return SUCCESS;
}

int GcFile::save()
{
  int ret = SUCCESS;

  if (NULL == file_op_)
  {
    //LOG(ERROR, "save fail, file not initialized");
    ret = ERROR;
  }
  else if (static_cast<int>(seg_info_.size()) > GC_BATCH_WIRTE_COUNT)
  {
    ret = save_gc();
  }
  return ret;
}

int GcFile::save_gc()
{
  int ret = SUCCESS;
  int32_t size = seg_info_.size() * sizeof(SegmentInfo);
  char* buf = new char[size];
  dump(buf, size);
  // to avoid the info conflict caused when fail between writing segment info and flushing segment head,
  // use pwrite innstead of write with append
  if ((ret = file_op_->pwrite_file(buf, size, file_pos_)) != SUCCESS)
  {
    //LOG(ERROR, "gc save fail, write file error, ret: %d", ret);
  }
  else if ((ret = file_op_->pwrite_file(reinterpret_cast<char*>(&seg_head_), sizeof(SegmentHead), 0)) != SUCCESS)
  {
    //LOG(ERROR, "gc flush head fail, ret: %d", ret);
  }
  else                        // write fail, not clear, wait for next chance
  {
    //LOG(DEBUG, "gc save segment success, count: %zd, raw size: %d, need gc segment count: %d, size: %"PRI64_PREFIX"d",
    //          seg_info_.size(), size, seg_head_.count_, seg_head_.size_);
    file_op_->flush_file();
    file_pos_ += size;
    clear_info();
  }
  gDeleteA(buf);

  return ret;
}

int GcFile::load()
{
  int ret = SUCCESS;
  if (NULL == file_op_)
  {
    //LOG(ERROR, "load fail, not initialize");
    ret = ERROR;
  }
  else if ((ret = load_head()) != SUCCESS)
  {
    //LOG(ERROR, "load segment head fail, ret: %d", ret);
  }
  else
  {
    char* buf = new char[sizeof(SegmentInfo) * seg_head_.count_];

    if ((ret = file_op_->pread_file(buf, sizeof(SegmentInfo) * seg_head_.count_, sizeof(SegmentHead))) != SUCCESS)
    {
      //LOG(ERROR, "load segment info fail, ret: %d", ret);
    }
    else
    {
      clear_info();
      SegmentInfo* seg_info = reinterpret_cast<SegmentInfo*>(buf);
      for (int32_t i = 0; i < seg_head_.count_; i++)
      {
        seg_info_.push_back(seg_info[i]);
        //LOG(DEBUG, "load segment info, offset: %"PRI64_PREFIX"d, blockid: %u, fileid: %"PRI64_PREFIX"u, size: %d, crc: %u", seg_info[i].offset_, seg_info[i].block_id_, seg_info[i].file_id_, seg_info[i].size_, seg_info[i].crc_);
      }
    }
    gDeleteA(buf);
  }
  return ret;
}

int GcFile::load_head()
{
  // check size 0 ?
  return file_op_->pread_file(reinterpret_cast<char*>(&seg_head_), sizeof(SegmentHead), 0);
}
