#ifndef N_DFS_UTIL_LOCAL_PACKET_H_
#define N_DFS_UTIL_LOCAL_PACKET_H_

#include "base_packet.h"
#include "new_client.h"
#include "client_manager.h"

namespace neptune {
namespace dfs {

class LocalPacket: public BasePacket
{
 public:
  LocalPacket(): new_client_(NULL) { setPCode(LOCAL_PACKET);}
  virtual ~LocalPacket()
  {
    if (NULL != new_client_)
    {
      NewClientManager::free_new_client_object(new_client_);
    }
  }
  bool copy(LocalPacket* , const int32_t , const bool ) { return false;}
  int serialize(Stream& ) const {return SUCCESS;}
  int deserialize(Stream& ) {return SUCCESS;}
  int64_t length() const { return 0;}

  inline int execute()
  {
    int32_t iret = NULL != new_client_ ? SUCCESS : ERROR;
    if (SUCCESS == iret)
    {
      new_client_->callback_(new_client_);
    }
    return iret;
  }
  void set_new_client(NewClient* new_client) { new_client_ = new_client;}
  NewClient* get_new_client() const { return new_client_;}
 
 private:
  DISALLOW_COPY_AND_ASSIGN(LocalPacket);
  NewClient* new_client_;
};

} //namespace dfs
} //namespace neptune

#endif
