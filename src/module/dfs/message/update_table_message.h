#ifndef N_DFS_MESSAGE_RTS_UPDATE_TABLE_MESSAGE_H
#define N_DFS_MESSAGE_RTS_UPDATE_TABLE_MESSAGE_H

#include "dfs/util/rts_define.h"
#include "dfs/util/base_packet.h"

namespace neptune {
namespace dfs {


using namespace neptune::dfs;

class UpdateTableMessage: public BasePacket 
{
 public:
  UpdateTableMessage();
  virtual ~UpdateTableMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  char* alloc(const int64_t length);
  inline char* get_table() { return tables_;}
  inline int64_t get_table_length() { return length_;}
  inline int64_t get_version() const { return version_;}
  inline void set_version(const int64_t version){ version_ = version;}
  inline int8_t get_phase(void) const { return phase_;}
  inline void set_phase(const int8_t phase) { phase_ = phase;}
private:
  char* tables_;
  int64_t length_;
  int64_t version_;
  int8_t  phase_;
  bool alloc_;
};

class UpdateTableResponseMessage: public BasePacket 
{
public:
  UpdateTableResponseMessage();
  virtual ~UpdateTableResponseMessage();
  virtual int serialize(Stream& output) const ;
  virtual int deserialize(Stream& input);
  virtual int64_t length() const;
  inline int64_t get_version() const { return version_;}
  inline void set_version(const int64_t version){ version_ = version;}
  inline int8_t get_phase(void) const { return phase_;}
  inline void set_phase(const int8_t phase) { phase_ = phase;}
  inline int8_t get_status() const { return status_;}
  inline void set_status(const int8_t status) { status_ = status;}

 private:
  int64_t version_;
  int8_t  phase_;
  int8_t  status_;
};


} //namespace dfs
} //namespace neptune

#endif //N_DFS_MESSAGE_RTS_UPDATE_TABLE_MESSAGE_H
