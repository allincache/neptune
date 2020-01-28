#ifndef N_DFS_DS_BLOCKSTATUS_H
#define N_DFS_DS_BLOCKSTATUS_H

#include "dfs/util/dfs.h"

namespace neptune {
namespace dfs {
namespace dataserver {

class BlockStatus
{
 public:
  BlockStatus()
  {
    reset();
  }

  ~BlockStatus()
  {
  }

  inline void reset()
  {
    atomic_set(&crc_error_, 0);
    atomic_set(&eio_error_, 0);
    atomic_set(&oper_warn_, 0);
  }

  inline int add_crc_error()
  {
    return atomic_add_return(1, &crc_error_);
  }

  inline int get_crc_error() const
  {
    return atomic_read(&crc_error_);
  }

  inline int add_eio_error()
  {
    return atomic_add_return(1, &eio_error_);
  }

  inline int get_eio_error() const
  {
    return atomic_read(&eio_error_);
  }

  inline int add_oper_warn()
  {
    return atomic_add_return(1, &oper_warn_);
  }

  inline int get_oper_warn() const
  {
    return atomic_read(&oper_warn_);
  }

 private:
  atomic_t crc_error_;
  atomic_t eio_error_;
  atomic_t oper_warn_;
};

} //namespace dataserver
} //namespace dfs
} //namespace neptune

#endif //N_DFS_DS_BLOCKSTATUS_H
