#ifndef NEP_BASE_COMMON_HANDLE_H
#define NEP_BASE_COMMON_HANDLE_H

#include "Exception.h"
#include <algorithm>

namespace neptune {
namespace base {

template<typename T>
class HandleBase
{
 public:
  typedef T element_type;
  
  T* get() const
  {
    return _ptr;
  }

  T* operator->() const
  {
    if(!_ptr)
    {
      throwNullHandleException(__FILE__, __LINE__);          
    }
    return _ptr;
  }

  T& operator*() const
  {
    if(!_ptr)
    {
      throwNullHandleException(__FILE__, __LINE__);           
    }
    return *_ptr;
  }

  operator bool() const
  {
    return _ptr ? true : false;
  }

  void swap(HandleBase& other)
  {
    std::swap(_ptr, other._ptr);
  }

  T* _ptr;

 private:
  void throwNullHandleException(const char *, int) const;
};

template<typename T> inline void 
HandleBase<T>::throwNullHandleException(const char* file, int line) const
{
#ifdef _NO_EXCEPTION
  assert(!"NullHandleException");
#else
  throw NullHandleException(file, line);
#endif
}

template<typename T, typename U>
inline bool operator==(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
  T* l = lhs.get();
  U* r = rhs.get();
  if(l && r)
  {
    return *l == *r;
  }
  return !l && !r;
}

template<typename T, typename U>
inline bool operator!=(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
  return !operator==(lhs, rhs);
}

template<typename T, typename U>
inline bool operator<(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
  T* l = lhs.get();
  U* r = rhs.get();
  if(l && r)
  {
    return *l < *r;
  }

  return !l && r;
}

template<typename T, typename U>
inline bool operator<=(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
  return lhs < rhs || lhs == rhs;
}

template<typename T, typename U>
inline bool operator>(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
  return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator>=(const HandleBase<T>& lhs, const HandleBase<U>& rhs)
{
  return !(lhs < rhs);
}

template<typename T>
class Handle : public HandleBase<T>
{
 public:   
  Handle(T* p = 0)
  {
    this->_ptr = p;
    if(this->_ptr)
    {
      this->_ptr->__incRef();
    }
  }
  
  template<typename Y>
  Handle(const Handle<Y>& r)
  {
    this->_ptr = r._ptr;
    if(this->_ptr)
    {
      this->_ptr->__incRef();
    }
  }

  Handle(const Handle& r)
  {
    this->_ptr = r._ptr;
    if(this->_ptr)
    {
      this->_ptr->__incRef();
    }
  }
  
  ~Handle()
  {
    if(this->_ptr)
    {
      this->_ptr->__decRef();
    }
  }
  
  Handle& operator=(T* p)
  {
    if(this->_ptr != p)
    {
      if(p)
      {
        p->__incRef();
      }
      T* ptr = this->_ptr;
      this->_ptr = p;
      if(ptr)
      {
        ptr->__decRef();
      }
    }
    return *this;
  }
      
  template<typename Y>
  Handle& operator=(const Handle<Y>& r)
  {
    if(this->_ptr != r._ptr)
    {
      if(r._ptr)
      {
        r._ptr->__incRef();
      }
      T* ptr = this->_ptr;
      this->_ptr = r._ptr;
      if(ptr)
      {
        ptr->__decRef();
      }
    }
    return *this;
  }

  Handle& operator=(const Handle& r)
  {
    if(this->_ptr != r._ptr)
    {
      if(r._ptr)
      {
        r._ptr->__incRef();
      }
      T* ptr = this->_ptr;
      this->_ptr = r._ptr;
      if(ptr)
      {
        ptr->__decRef();
      }
    }
    return *this;
  }
      
  template<class Y>
  static Handle dynamicCast(const HandleBase<Y>& r)
  {
    return Handle(dynamic_cast<T*>(r._ptr));
  }

  template<class Y>
  static Handle dynamicCast(Y* p)
  {
    return Handle(dynamic_cast<T*>(p));
  }
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_COMMON_HANDLE_H
