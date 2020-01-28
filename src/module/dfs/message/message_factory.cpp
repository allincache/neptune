#include "message_factory.h"

namespace neptune {
namespace dfs {

Packet* MessageFactory::createPacket(int pcode)
{
  Packet* packet = BasePacketFactory::createPacket(pcode);
  if (NULL == packet)
  {
    int32_t real_pcode = pcode & 0xFFFF;
    switch (real_pcode)
    {
      case GET_BLOCK_INFO_MESSAGE:
        packet = new GetBlockInfoMessage();
        break;
      case SET_BLOCK_INFO_MESSAGE:
        packet = new  SetBlockInfoMessage();
        break;
      case BATCH_GET_BLOCK_INFO_MESSAGE:
        packet = new  BatchGetBlockInfoMessage();
        break;
      case BATCH_SET_BLOCK_INFO_MESSAGE:
        packet = new  BatchSetBlockInfoMessage();
        break;
      case CARRY_BLOCK_MESSAGE:
        packet = new  CarryBlockMessage();
        break;
      case SET_DATASERVER_MESSAGE:
        packet = new  SetDataserverMessage();
        break;
      case UPDATE_BLOCK_INFO_MESSAGE:
        packet = new  UpdateBlockInfoMessage();
        break;
      case BLOCK_WRITE_COMPLETE_MESSAGE:
        packet = new  BlockWriteCompleteMessage();
        break;
      case READ_DATA_MESSAGE:
        packet = new  ReadDataMessage();
        break;
      case RESP_READ_DATA_MESSAGE:
        packet = new  RespReadDataMessage();
        break;
      case FILE_INFO_MESSAGE:
        packet = new  FileInfoMessage();
        break;
      case RESP_FILE_INFO_MESSAGE:
        packet = new  RespFileInfoMessage();
        break;
      case WRITE_DATA_MESSAGE:
        packet = new  WriteDataMessage();
        break;
      case CLOSE_FILE_MESSAGE:
        packet = new  CloseFileMessage();
        break;
      case UNLINK_FILE_MESSAGE:
        packet = new  UnlinkFileMessage();
        break;
      case REPLICATE_BLOCK_MESSAGE:
        packet = new  ReplicateBlockMessage();
        break;
      case COMPACT_BLOCK_MESSAGE:
        packet = new  CompactBlockMessage();
        break;
      case GET_SERVER_STATUS_MESSAGE:
        packet = new  GetServerStatusMessage();
        break;
      /*case SUSPECT_DATASERVER_MESSAGE:
        packet = new  SuspectDataserverMessage();
        break;*/
      case RENAME_FILE_MESSAGE:
        packet = new  RenameFileMessage();
        break;
      case CLIENT_CMD_MESSAGE:
        packet = new  ClientCmdMessage();
        break;
      case CREATE_FILENAME_MESSAGE:
        packet = new  CreateFilenameMessage();
        break;
      case RESP_CREATE_FILENAME_MESSAGE:
        packet = new  RespCreateFilenameMessage();
        break;
      case ROLLBACK_MESSAGE:
        packet = new  RollbackMessage();
        break;
      case RESP_HEART_MESSAGE:
        packet = new  RespHeartMessage();
        break;
      case RESET_BLOCK_VERSION_MESSAGE:
        packet = new  ResetBlockVersionMessage();
        break;
      case BLOCK_FILE_INFO_MESSAGE:
        packet = new  BlockFileInfoMessage();
        break;
      case NEW_BLOCK_MESSAGE:
        packet = new  NewBlockMessage();
        break;
      case REMOVE_BLOCK_MESSAGE:
        packet = new  RemoveBlockMessage();
        break;
      case LIST_BLOCK_MESSAGE:
        packet = new  ListBlockMessage();
        break;
      case RESP_LIST_BLOCK_MESSAGE:
        packet = new  RespListBlockMessage();
        break;
      case BLOCK_RAW_META_MESSAGE:
        packet = new  BlockRawMetaMessage();
        break;
      case WRITE_RAW_DATA_MESSAGE:
        packet = new  WriteRawDataMessage();
        break;
      case WRITE_INFO_BATCH_MESSAGE:
        packet = new  WriteInfoBatchMessage();
        break;
      case BLOCK_COMPACT_COMPLETE_MESSAGE:
        packet = new  CompactBlockCompleteMessage();
        break;
      case READ_DATA_MESSAGE_V2:
        packet = new  ReadDataMessageV2();
        break;
      case RESP_READ_DATA_MESSAGE_V2:
        packet = new  RespReadDataMessageV2();
        break;
      case LIST_BITMAP_MESSAGE:
        packet = new  ListBitMapMessage();
        break;
      case RESP_LIST_BITMAP_MESSAGE:
        packet = new  RespListBitMapMessage();
        break;
      case RELOAD_CONFIG_MESSAGE:
        packet = new  ReloadConfigMessage();
        break;
      case READ_RAW_DATA_MESSAGE:
        packet = new  ReadRawDataMessage();
        break;
      case RESP_READ_RAW_DATA_MESSAGE:
        packet = new  RespReadRawDataMessage();
        break;
      case ACCESS_STAT_INFO_MESSAGE:
        packet = new  AccessStatInfoMessage();
        break;
      case READ_SCALE_IMAGE_MESSAGE:
        packet = new  ReadScaleImageMessage();
        break;
      case CRC_ERROR_MESSAGE:
        packet = new  CrcErrorMessage();
        break;
      case OPLOG_SYNC_MESSAGE:
        packet = new  OpLogSyncMessage();
        break;
      case OPLOG_SYNC_RESPONSE_MESSAGE:
        packet = new  OpLogSyncResponeMessage();
        break;
      case MASTER_AND_SLAVE_HEART_MESSAGE:
        packet = new  MasterAndSlaveHeartMessage();
        break;
      case MASTER_AND_SLAVE_HEART_RESPONSE_MESSAGE:
        packet = new  MasterAndSlaveHeartResponseMessage();
        break;
      case HEARTBEAT_AND_NS_HEART_MESSAGE:
        packet = new  HeartBeatAndNSHeartMessage();
        break;
      case ADMIN_CMD_MESSAGE:
        packet = new  AdminCmdMessage();
        break;
      case REMOVE_BLOCK_RESPONSE_MESSAGE:
        packet = new  RemoveBlockResponseMessage();
        break;
      case DUMP_PLAN_MESSAGE:
        packet = new  DumpPlanMessage();
        break;
      case DUMP_PLAN_RESPONSE_MESSAGE:
        packet = new  DumpPlanResponseMessage();
        break;
      case SHOW_SERVER_INFORMATION_MESSAGE:
        packet = new  ShowServerInformationMessage();
        break;
      case REQ_RC_LOGIN_MESSAGE:
        packet = new ReqRcLoginMessage();
        break;
      case RSP_RC_LOGIN_MESSAGE:
        packet = new RspRcLoginMessage();
        break;
      case REQ_RC_KEEPALIVE_MESSAGE:
        packet = new ReqRcKeepAliveMessage();
        break;
      case RSP_RC_KEEPALIVE_MESSAGE:
        packet = new RspRcKeepAliveMessage();
        break;
      case REQ_RC_LOGOUT_MESSAGE:
        packet = new ReqRcLogoutMessage();
        break;
      case REQ_RC_RELOAD_MESSAGE:
        packet = new ReqRcReloadMessage();
        break;
      case GET_DATASERVER_INFORMATION_MESSAGE:
        packet = new GetDataServerInformationMessage();
        break;
      case GET_DATASERVER_INFORMATION_RESPONSE_MESSAGE:
        packet = new GetDataServerInformationResponseMessage();
        break;
      case FILEPATH_ACTION_MESSAGE:
        packet = new FilepathActionMessage();
        break;
      case WRITE_FILEPATH_MESSAGE:
        packet = new WriteFilepathMessage();
        break;
      case READ_FILEPATH_MESSAGE:
        packet = new ReadFilepathMessage();
        break;
      case RESP_READ_FILEPATH_MESSAGE:
        packet = new RespReadFilepathMessage();
        break;
      case LS_FILEPATH_MESSAGE:
        packet = new LsFilepathMessage();
        break;
      case RESP_LS_FILEPATH_MESSAGE:
        packet = new RespLsFilepathMessage();
        break;
      case REQ_RT_MS_KEEPALIVE_MESSAGE:
        packet =  new RtsMsHeartMessage();
        break;
      case RSP_RT_MS_KEEPALIVE_MESSAGE:
        packet = new RtsMsHeartResponseMessage();
        break;
      case REQ_RT_RS_KEEPALIVE_MESSAGE:
        packet =  new RtsRsHeartMessage();
        break;
      case RSP_RT_RS_KEEPALIVE_MESSAGE:
        packet = new RtsRsHeartResponseMessage();
        break;
      case REQ_RT_GET_TABLE_MESSAGE:
        packet = new GetTableFromRtsMessage();
        break;
      case RSP_RT_GET_TABLE_MESSAGE:
        packet = new GetTableFromRtsResponseMessage();
        break;
      case REQ_RT_UPDATE_TABLE_MESSAGE:
        packet = new UpdateTableMessage();
        break;
      case RSP_RT_UPDATE_TABLE_MESSAGE:
        packet = new UpdateTableResponseMessage();
        break;
      case REQ_CALL_DS_REPORT_BLOCK_MESSAGE:
        packet = new CallDsReportBlockRequestMessage();
        break;
      case REQ_REPORT_BLOCKS_TO_NS_MESSAGE:
        packet = new ReportBlocksToNsRequestMessage();
        break;
      case RSP_REPORT_BLOCKS_TO_NS_MESSAGE:
        packet = new ReportBlocksToNsResponseMessage();
        break;
      case REQ_CHECK_BLOCK_MESSAGE:
        packet = new CheckBlockRequestMessage();
        break;
      case RSP_CHECK_BLOCK_MESSAGE:
        packet = new CheckBlockResponseMessage();
        break;
      default:
        LOG(ERROR, "pcode: %d not found in message factory", real_pcode);
        break;
    }
  }
  return packet;
}

} //namespace dfs
} //namespace neptune
