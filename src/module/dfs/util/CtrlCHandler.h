#ifndef TBSYS_CTRL_C_HANDLER_H
#define TBSYS_CTRL_C_HANDLER_H

#include "dfs/util/dfs.h"

namespace neptune {
namespace dfs {

typedef void (*CtrlCHandlerCallback)(int);

class CtrlCHandler
{

 public:
  CtrlCHandler(CtrlCHandlerCallback = 0);
  ~CtrlCHandler();
  void setCallback(CtrlCHandlerCallback callback);
  CtrlCHandlerCallback getCallback() const;
};

class CtrlCHandlerException : public Exception
{ 
 public:
    CtrlCHandlerException(const char*, int);
    virtual std::string _name() const;
    virtual Exception* _clone() const;
    virtual void _throw() const;
};

} //namespace dfs
} //namespace neptune

#endif
