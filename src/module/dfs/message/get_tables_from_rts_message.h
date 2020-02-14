#ifndef N_DFS_MESSAGE_RTS_GET_TABLES_MESSAGE_H
#define N_DFS_MESSAGE_RTS_GET_TABLES_MESSAGE_H

#include "dfs/util/rts_define.h"
#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {

using namespace neptune::dfs;

class GetTableFromRtsResponseMessage: public BasePacket 
{
 public:
  GetTableFromRtsResponseMessage();
  virtual ~GetTableFromRtsResponseMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  char* alloc(const int64_t length);
  inline char* get_table() { return tables_;}
  inline int64_t& get_table_length() { return length_;}
  inline int64_t& get_version() { return version_;}
 private:
  char* tables_;
  int64_t length_;
  int64_t version_;
};

class GetTableFromRtsMessage: public BasePacket 
{
 public:
  GetTableFromRtsMessage();
  virtual ~GetTableFromRtsMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
 private:
  int8_t  reserve_;
};

} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_RTS_GET_TABLES_MESSAGE_H
