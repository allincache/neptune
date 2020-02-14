#ifndef N_DFS_UTIL_STATUS_PACKET_H_
#define N_DFS_UTIL_STATUS_PACKET_H_

#include "base_packet.h"

namespace neptune {
namespace dfs {

class StatusMessage: public BasePacket 
{
 public:
  StatusMessage();
  StatusMessage(const int32_t status, const char* const str = NULL);
  void set_message(const int32_t status, const char* const str = NULL);
  virtual ~StatusMessage();
  int serialize(Stream& output) const;
  int deserialize(Stream& input);
  int64_t length() const;
  inline const char* get_error() const { return msg_;}
  inline int64_t get_error_msg_length() const { return length_;}
  inline int32_t get_status() const { return status_;}

 private:
  char msg_[MAX_ERROR_MSG_LENGTH + 1];/** include '\0'*/
  int64_t length_;
  int32_t status_;
};

} //namespace dfs
} //namespace neptune

#endif
