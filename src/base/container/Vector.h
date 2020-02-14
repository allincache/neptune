#ifndef NEP_BASE_CONTAINER_VECTOR_H
#define NEP_BASE_CONTAINER_VECTOR_H

#include <algorithm>
#include <functional>
#include "base/common/Memory.h"

namespace neptune {
namespace base {

template <typename T> class base_vector_traits;

template <typename T>
struct base_vector_traits<T*>
{
  typedef T pointer;
  typedef T* value_type;
  typedef T* const const_value_type;
  typedef T** iterator;
};

template <typename T>
struct base_vector_traits<const T*>
{
  typedef T pointer;
  typedef const T* value_type;
  typedef const T* const const_value_type;
  typedef const T** iterator;
};

template <typename T >
class VectorHelper
{
 public:
  typedef typename base_vector_traits<T>::pointer pointer;
  typedef typename base_vector_traits<T>::value_type value_type;
  typedef typename base_vector_traits<T>::const_value_type const_value_type;
  typedef typename base_vector_traits<T>::iterator iterator;
  typedef int32_t size_type;

 public:
  VectorHelper(int32_t size, int32_t min_expand_size, float expand_ratio);
  virtual ~VectorHelper();

  inline iterator begin() const { return start_;}
  inline iterator end() const   { return finish_;}

  inline size_type size() const  { return end() - begin();}
  inline size_type capacity() const { return end_of_storage_ - begin();}
  inline size_type remain() const { return end_of_storage_ - end();}

  inline value_type operator[] (size_type index) const {return at(index);}

  inline value_type at(size_type index) const {range_check(index); return *(begin() + index);}

  int push_back(const_value_type value);

  int insert(iterator position, const_value_type value);

  value_type erase(iterator position);

  value_type erase(const_value_type value);

  iterator find(const_value_type value);

  bool empty() const { return  0 == size(); }

  int expand_ratio(const float expand_ratio = 0.1);

  bool need_expand(const float reserve_ratio = 0.1);

  void clear();

private:
  inline void range_check(size_type index) const {assert(index < size() && index >= 0);}
  static iterator fill(iterator dest, const_value_type value);
  static iterator copy(iterator dest, iterator begin, iterator end);
  static iterator move(iterator dest, iterator begin, iterator end);
  int expand(const int32_t expand_size);
private:
  iterator start_;
  iterator finish_;
  iterator end_of_storage_;
  float expand_ratio_;
  size_type min_expand_size_;
};

template <typename T, typename Compare = std::less<T> >
class BaseSortedVector
{
public:
  typedef VectorHelper<T> base_vector;
  typedef typename base_vector::pointer pointer;
  typedef typename base_vector::value_type value_type;
  typedef typename base_vector::const_value_type const_value_type;
  typedef typename base_vector::iterator iterator;
  typedef typename base_vector::size_type size_type;
public:
  BaseSortedVector(int32_t size, int32_t min_expand_size, float expand_ratio)
  : storage_(size, min_expand_size,expand_ratio){}
  virtual ~BaseSortedVector(){}

  inline iterator begin() const { return storage_.begin();}
  inline iterator end() const   { return storage_.end();}

  inline size_type size() const  { return storage_.size();}
  inline size_type capacity() const { return storage_.capacity();}
  inline size_type remain() const { return storage_.remain();}

  inline value_type operator[] (size_type index) const {return storage_.at(index);}

  inline value_type at(size_type index) const {return storage_.at(index);}

  iterator find(const_value_type value) const;

  iterator lower_bound(const_value_type value) const;
  iterator upper_bound(const_value_type value) const;

  int insert(const_value_type value);
  int insert_unique(value_type& output, const_value_type value);
  int push_back(const_value_type value);

  value_type erase(const_value_type value);

  int expand_ratio(const float expand_ratio = 0.1);

  bool need_expand(const float expand_ratio = 0.1);

  void clear();

  bool empty() const { return storage_.empty(); }

 private:
  base_vector storage_;
  Compare comp_;
};

} //namespace base
} //namespace neptune

#include "VectorHelper.ipp"

#endif //NEP_BASE_CONTAINER_VECTOR_H