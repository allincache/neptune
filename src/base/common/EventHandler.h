#ifndef NEP_BASE_COMMON_EVENT_HANDLER_H
#define NEP_BASE_COMMON_EVENT_HANDLER_H

namespace neptune {
namespace base {

class ThreadPool;

class ThreadPoolWorkItem 
{
 public:
  virtual ~ThreadPoolWorkItem(){}
  virtual void destroy( )=0;
  virtual void execute( const ThreadPool* ) = 0;
};

}//namespace base
}//namespace neptune

#endif
