#ifndef N_DFS_DS_DATAMANAGEMENT_H_
#define N_DFS_DS_DATAMANAGEMENT_H_

#include <vector>
#include <map>
#include <list>
#include "logic_block.h"
#include "dataserver_define.h"
#include "data_file.h"
#include "dfs/util/parameter.h"
#include "base/concurrent/Mutex.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class DataManagement
{
  public:
    DataManagement();
    ~DataManagement();

  public:
    void set_file_number(const uint64_t file_number);
    int init_block_files(const FileSystemParameter& fs_param);
    void get_ds_filesystem_info(int32_t& block_count, int64_t& use_capacity, int64_t& total_capacity);
    int get_all_logic_block(std::list<LogicBlock*>& logic_block_list);
    int get_all_block_info(std::set<BlockInfo>& blocks);
    int64_t get_all_logic_block_size();

    int create_file(const uint32_t block_id, uint64_t& file_id, uint64_t& file_number);
    int write_data(const WriteDataInfo& write_info, const int32_t lease_id, int32_t& version,
        const char* data_buffer, UpdateBlockType& repair);
    int erase_data_file(const uint64_t file_number);
    int close_write_file(const CloseFileInfo& close_file_info, int32_t& write_file_size);
    int read_data(const uint32_t block_id, const uint64_t file_id, const int32_t read_offset, const int8_t flag,
        int32_t& real_read_len, char* tmpDataBuffer);
    int read_raw_data(uint32_t block_id, int32_t read_offset, int32_t& real_read_len, char* tmpDataBuffer);

    int read_file_info(const uint32_t block_id,
        const uint64_t file_id, const int32_t mode, FileInfo& finfo);
    int rename_file(const uint32_t block_id, const uint64_t file_id, const uint64_t new_file_id);
    int unlink_file(const uint32_t block_id, const uint64_t file_id, const int32_t action, int64_t& file_size);

    int batch_new_block(const VUINT32* new_blocks);
    int batch_remove_block(const VUINT32* remove_blocks);

    int query_bit_map(const int32_t query_type, char** tmp_data_buffer, int32_t& bit_map_len, int32_t& set_count);

    int query_block_status(const int32_t query_type, VUINT& block_ids, std::map<uint32_t, std::vector<
        uint32_t> >& logic_2_physic_blocks, std::map<uint32_t, BlockInfo*>& block_2_info);
    int get_block_info(const uint32_t block_id, BlockInfo*& blk, int32_t& visit_count);

    int get_visit_sorted_blockids(std::vector<LogicBlock*>& block_ptrs);
    int get_block_file_list(const uint32_t block_id, std::vector<FileInfo>& fileinfos);
    int get_block_meta_info(const uint32_t block_id, RawMetaVec& meta_list);
    int reset_block_version(const uint32_t block_id);

    int new_single_block(const uint32_t block_id, const BlockType type = C_MAIN_BLOCK);
    int del_single_block(const uint32_t block_id);
    int get_block_curr_size(const uint32_t block_id, int32_t& size);
    int write_raw_data(const uint32_t block_id, const int32_t data_offset, const int32_t msg_len,
        const char* data_buffer);
    int batch_write_meta(const uint32_t block_id, const BlockInfo* blk,
        const RawMetaVec* meta_list);

    int add_new_expire_block(const VUINT32* expire_block_ids, const VUINT32* remove_block_ids,
        const VUINT32* new_block_ids);

    //gc thread
    int gc_data_file();
    int remove_data_file();

  private:
    DISALLOW_COPY_AND_ASSIGN(DataManagement);

    uint64_t file_number_;          // file id
    Mutex data_file_mutex_; // datafile mutex
    DataFileMap data_file_map_; // datafile map

    //gc datafile
    int32_t last_gc_data_file_time_; // last datafile gc time
    RWLock block_rw_lock_;   // block layer read-write lock
};

struct visit_count_sort
{
    bool operator()(const LogicBlock *x, const LogicBlock *y) const
    {
      return (x->get_visit_count() > y->get_visit_count());
    }
};

}
}
}

#endif //N_DFS_DS_DATAMANAGEMENT_H_
