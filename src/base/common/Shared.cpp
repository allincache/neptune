#include <cassert>
#include "Shared.h"

namespace neptune {
namespace base {

SimpleShared::SimpleShared() :
  _ref(0),
  _noDelete(false)
{
}

SimpleShared::SimpleShared(const SimpleShared&) :
  _ref(0),
  _noDelete(false)
{
}

Shared::Shared() :
  _ref(0),
  _noDelete(false)
{
}

Shared::Shared(const Shared&) :
  _ref(0),
  _noDelete(false)
{
}

void Shared::__incRef()
{
  _mutex.lock();
  ++_ref;
  _mutex.unlock();
}

void Shared::__decRef()
{
  _mutex.lock();
  bool doDelete = false;
  assert(_ref > 0);
  if(--_ref == 0)
  {
    doDelete = !_noDelete;
    _noDelete = true;
  }
  _mutex.unlock();
  if(doDelete)
  {
    delete this;
  }
}

int Shared::__getRef() const
{
  _mutex.lock();
  const int ref = _ref;
  _mutex.unlock();
  return ref;
}

void Shared::__setNoDelete(bool b)
{
  _noDelete = b;
}

} //namespace base
} //namespace neptune
