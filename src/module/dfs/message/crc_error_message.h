#ifndef N_DFS_MESSAGE_CRCERRORMESSAGE_H
#define N_DFS_MESSAGE_CRCERRORMESSAGE_H

#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class CrcErrorMessage: public BasePacket 
{
 public:
  CrcErrorMessage();
  virtual ~CrcErrorMessage();

  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline void set_block_id(const uint32_t block_id) { block_id_ = block_id;}
  inline uint32_t get_block_id() const { return block_id_;}
  inline void set_file_id(const uint64_t file_id) { file_id_ = file_id;}
  inline uint64_t get_file_id() const { return file_id_;}
  inline void set_crc(const uint32_t crc) {crc_ = crc;}
  inline uint32_t get_crc() const { return crc_;}
  inline void set_error_flag(const CheckDsBlockType flag){error_flag_ = flag;}
  inline CheckDsBlockType get_error_flag() const { return error_flag_; }
  inline void add_fail_server(const uint64_t server_id){fail_server_.push_back(server_id); }
  inline const VUINT64* get_fail_server() const{return &fail_server_;}
 protected:
  uint32_t block_id_;
  uint64_t file_id_;
  uint32_t crc_; // client report the correct crc
  CheckDsBlockType error_flag_;
  VUINT64 fail_server_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_CRCERRORMESSAGE_H
