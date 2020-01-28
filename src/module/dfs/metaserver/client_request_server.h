#ifndef N_DFS_MS_CLIENT_REQUEST_SERVER_H_
#define N_DFS_MS_CLIENT_REQUEST_SERVER_H_

#include "base/network/simple/net.h"
#include "base/concurrent/Lock.h"
#include "block_collect.h"
#include "server_collect.h"
#include "dfs/util/base_packet.h"
#include "oplog_sync_manager.h"

namespace neptune {
namespace dfs {
namespace metaserver {

struct CloseParameter
{
  BlockInfo block_info_;
  uint64_t id_;
  uint32_t lease_id_;
  UnlinkFlag unlink_flag_;
  WriteCompleteStatus status_;
  bool need_new_;
  char error_msg_[256];
};

class LayoutManager;
class MetaServer;

class ClientRequestServer
{
 public:
  explicit ClientRequestServer(LayoutManager& manager);
  virtual ~ClientRequestServer(){}

  int keepalive(const DataServerStatInfo& info, const time_t now);
  int report_block(const uint64_t server, const time_t now, std::set<BlockInfo>& blocks);
  int open(uint32_t& block_id, uint32_t& lease_id, int32_t& version,
      VUINT64& servers, const int32_t mode, const time_t now);
  int batch_open(const VUINT32& blocks, const int32_t mode, const int32_t block_count, std::map<uint32_t, BlockInfoSeg>& out);

  int close(CloseParameter& param);

  void dump_plan(DataBuffer& output);

  int handle_control_cmd(const ClientCmdInformation& info, BasePacket* msg, const int64_t buf_length, char* buf);

  int handle(BasePacket* msg);

 private:
  int open_read_mode_(VUINT64& servers, const uint32_t block) const;
  int open_write_mode_(uint32_t& block_id, uint32_t& lease_id,
      int32_t& version, VUINT64& servers, const int32_t mode, const time_t now);
  int batch_open_read_mode_(std::map<uint32_t, BlockInfoSeg>& out, const VUINT32& blocks) const;
  int batch_open_write_mode_(std::map<uint32_t, BlockInfoSeg>& out,const int32_t mode, const int32_t block_count);

  int  handle_control_load_block(const time_t now, const ClientCmdInformation& info, BasePacket* message, const int64_t buf_length, char* error_buf);
  int  handle_control_delete_block(const time_t now, const ClientCmdInformation& info,const int64_t buf_length, char* error_buf);
  int  handle_control_compact_block(const time_t now, const ClientCmdInformation& info, const int64_t buf_length, char* error_buf);
  int  handle_control_immediately_replicate_block(const time_t now, const ClientCmdInformation& info, const int64_t buf_length, char* error_buf);
  int  handle_control_rotate_log(void);
  int  handle_control_set_runtime_param(const ClientCmdInformation& info, const int64_t buf_length, char* error_buf);
  int  handle_control_get_balance_percent(const int64_t buf_length, char* error_buf);
  int  handle_control_set_balance_percent(const ClientCmdInformation& info, const int64_t buf_length, char* error_buf);
  int  handle_control_clear_system_table(const ClientCmdInformation& info, const int64_t buf_length, char* error_buf);

  bool is_discard(void);

 private:
  volatile uint64_t ref_count_;
  LayoutManager& manager_;
};

}
}
}

#endif

