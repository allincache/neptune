#ifndef NEP_BASE_COMMON_SHARED_H 
#define NEP_BASE_COMMON_SHARED_H 

#include "base/concurrent/Mutex.h"

namespace neptune {
namespace base {

class SimpleShared
{
 public:

  SimpleShared();
  SimpleShared(const SimpleShared&);

  virtual ~SimpleShared()
  {
  }

  SimpleShared& operator=(const SimpleShared&)
  {
    return *this;
  }

  void __incRef()
  {
    assert(_ref >= 0);
    ++_ref;
  }

  void __decRef()
  {
    assert(_ref > 0);
    if(--_ref == 0)
    {
      if(!_noDelete)
      {
        _noDelete = true;
        delete this;
      }
    }
  }

  int __getRef() const
  {
    return _ref;
  }

  void __setNoDelete(bool b)
  {
    _noDelete = b;
  }

 private:

  int _ref;
  bool _noDelete;
};

class Shared
{
 public:

  Shared();
  Shared(const Shared&);

  virtual ~Shared()
  {
  }

  Shared& operator=(const Shared&)
  {
      return *this;
  }

  virtual void __incRef();

  virtual void __decRef();

  virtual int __getRef() const;

  virtual void __setNoDelete(bool);

 protected:
  int _ref;
  bool _noDelete;
  Mutex _mutex;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_COMMON_SHARED_H
