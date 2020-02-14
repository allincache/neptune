#ifndef N_DFS_MESSAGE_MESSAGEFACTORY_H
#define N_DFS_MESSAGE_MESSAGEFACTORY_H

#include "dfs/util/base_packet.h"
#include "dfs/util/base_packet_factory.h"
#include "dataserver_message.h"
#include "block_info_message.h"
#include "close_file_message.h"
#include "read_data_message.h"
#include "write_data_message.h"
#include "unlink_file_message.h"
#include "replicate_block_message.h"
#include "compact_block_message.h"
#include "server_status_message.h"
#include "file_info_message.h"
#include "rename_file_message.h"
#include "client_cmd_message.h"
#include "create_filename_message.h"
#include "rollback_message.h"
#include "heart_message.h"
#include "reload_message.h"
#include "oplog_sync_message.h"
#include "crc_error_message.h"
#include "admin_cmd_message.h"
#include "dump_plan_message.h"
#include "rc_session_message.h"
#include "get_dataserver_information_message.h"
#include "metaserver_client_message.h"
#include "rts_ms_heart_message.h"
#include "rts_rts_heart_message.h"
#include "get_tables_from_rts_message.h"
#include "update_table_message.h"
#include "checkserver_message.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class MessageFactory: public BasePacketFactory
{
 public:
  MessageFactory(){}
  virtual ~MessageFactory(){}
  virtual Packet* createPacket(int pcode);
};
  
} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_MESSAGEFACTORY_H
