#include "base/common/Define.h"
#include "base/common/ErrorMsg.h"

namespace neptune {
namespace base {

template <typename T>
VectorHelper<T>::VectorHelper(int32_t size, int32_t expand_size, float expand_ratio):
  start_(new T[size]),
  finish_(start_),
  end_of_storage_(start_ + size),
  expand_ratio_(expand_ratio),
  min_expand_size_(expand_size)
{
  assert(size > 0);
}

template <typename T>
VectorHelper<T>::~VectorHelper()
{
  gDeleteA(start_);
}

template <typename T>
int VectorHelper< T >::push_back(const_value_type value)
{
  int32_t ret = SUCCESS;
  if (finish_ != end_of_storage_)
  {
    memcpy(finish_, &value, sizeof(value));
    ++finish_;
  }
  else
  {
    ret = insert(end(), value);
  }
  return ret;
}

template < typename T >
typename VectorHelper<T>::iterator VectorHelper<T>::fill(iterator dest, const_value_type value)
{
  if (0 != dest)
  {
    *dest = value;
  }
  return dest;
}

template <typename T>
typename VectorHelper<T>::iterator VectorHelper<T>::copy(iterator dest, iterator start, iterator finish)
{
  assert(dest);
  assert(finish);
  assert(start);
  assert(finish>= start);
  int32_t n = finish - start;
  if (n > 0)
    ::memcpy(dest, start, n * sizeof(value_type));
  return dest + n;
}

template <typename T>
typename VectorHelper<T>::iterator VectorHelper<T>::move(iterator dest, iterator start, iterator finish)
{
  assert(dest);
  assert(finish);
  assert(start);
  assert(finish >= start);
  int32_t n = finish - start;
  if (n > 0)
    ::memmove(dest, start, n * sizeof(value_type));
  return dest + n;
}

template <typename T>
int VectorHelper<T>::insert(iterator position, const_value_type value)
{
  int32_t ret = SUCCESS;
  if (remain() >= 1)
  {
    if (finish_ == position)
    {
      fill(finish_, value);
      ++finish_;
    }
    else
    {
      move(position + 1, position, finish_);
      fill(position, value);
      ++finish_;
    }
  }
  else
  {
    int32_t old_size = size();
    int32_t old_capacity = capacity();
    int32_t expand_size = static_cast<int32_t>(old_capacity * expand_ratio_);
    if (expand_size < min_expand_size_)
      expand_size = min_expand_size_;
    int32_t new_size = old_capacity + expand_size;

    iterator new_pointer = new T[new_size];
    ret = new_pointer ? SUCCESS : ERROR;
    if (SUCCESS == ret)
    {
      if (finish_ == position)
      {
        copy(new_pointer, start_, finish_);
        fill(new_pointer + old_size, value);
      }
      else
      {
        iterator current = copy(new_pointer, start_, position);
        fill(current, value);
        ++current;
        copy(current, position, finish_);
      }
      gDeleteA(start_);
      start_ = new_pointer;
      finish_ = new_pointer + old_size + 1;
      end_of_storage_ = new_pointer + new_size;
    }
  }
  return ret;
}

template <typename T>
typename VectorHelper<T>::value_type VectorHelper<T>::erase(iterator position)
{
  assert(position >= start_);
  assert(position <= finish_);
  move(position, position + 1, finish_);
  --finish_;
  return *position;
}

template <typename T>
typename VectorHelper<T>::value_type VectorHelper<T>::erase(const_value_type value)
{
  value_type return_value = NULL;
  iterator pos = std::find(start_, finish_, value);
  if (finish_ != pos)
  {
    return_value = (*pos);
    erase(pos);
  }
  return return_value;
}

template <typename T>
typename VectorHelper<T>::iterator VectorHelper<T>::find(const_value_type value)
{
  iterator pos = std::find(start_, finish_, value);
  return pos;
}

template <typename T>
int VectorHelper<T>::expand(int32_t expand_size)
{
  int32_t old_size = size();
  assert(expand_size >= old_size);
  iterator new_pointer = new T[expand_size];
  int32_t ret = new_pointer ? SUCCESS : ERROR;
  if (SUCCESS == ret)
  {
    copy(new_pointer, start_, finish_);
    gDeleteA(start_);
    start_ = new_pointer;
    finish_ = start_ + old_size;
    end_of_storage_ = start_ + expand_size;
  }
  return ret;
}

template <typename T>
int VectorHelper<T>::expand_ratio(const float expand_ratio)
{
  if (expand_ratio != expand_ratio_)
    expand_ratio_ = expand_ratio;

  int32_t old_capacity = capacity();
  int32_t new_expand_size = static_cast<int32_t>(expand_ratio * old_capacity);
  if (new_expand_size < min_expand_size_)
    new_expand_size = min_expand_size_;
  return expand(new_expand_size + old_capacity);
}

template <typename T>
bool VectorHelper<T>::need_expand(const float expand_ratio_ratio)
{
  float ratio = static_cast<float>(remain()) / capacity();
  return ratio < expand_ratio_ratio;
}

template <typename T>
void VectorHelper<T>::clear()
{
  finish_ = start_;
}

template <typename T, typename Compare>
typename BaseSortedVector<T,Compare>::iterator BaseSortedVector<T,Compare>::find(const_value_type value) const
{
  iterator pos = std::lower_bound(storage_.begin(), storage_.end(), value, comp_);
  if (pos != storage_.end())
  {
    if (comp_(*pos, value) || comp_(value, *pos))
      pos = storage_.end();
  }
  return pos;
}

template <typename T, typename Compare>
typename BaseSortedVector<T,Compare>::iterator BaseSortedVector<T,Compare>::lower_bound(const_value_type value) const
{
  return std::lower_bound(storage_.begin(), storage_.end(), value, comp_);
}

template <typename T, typename Compare>
typename BaseSortedVector<T,Compare>::iterator BaseSortedVector<T,Compare>::upper_bound(const_value_type value) const
{
  return std::upper_bound(storage_.begin(), storage_.end(), value, comp_);
}

template <typename T, typename Compare>
int BaseSortedVector<T, Compare>::insert(const_value_type value)
{
  iterator pos = std::lower_bound(storage_.begin(), storage_.end(), value, comp_);
  return storage_.insert(pos, value);
}

template <typename T, typename Compare>
int BaseSortedVector<T, Compare>::insert_unique(value_type& output, const_value_type value)
{
  int ret = SUCCESS;
  iterator pos = std::lower_bound(storage_.begin(), storage_.end(), value, comp_);
  if (storage_.end() != pos)
  {
    ret = (comp_(*pos, value) || comp_(value, *pos)) ? SUCCESS : EXIT_ELEMENT_EXIST; 
    if (EXIT_ELEMENT_EXIST == ret)
      output = *pos;
  }
  if (SUCCESS == ret)
  {
    ret = storage_.insert(pos, value);
    if (SUCCESS == ret)
      output = value;
  }
  return ret;
}

template <typename T, typename Compare>
int BaseSortedVector<T, Compare>::push_back(const_value_type value)
{
  storage_.push_back(value);
  return SUCCESS;
}

template <typename T, typename Compare>
typename BaseSortedVector<T, Compare>::value_type BaseSortedVector<T, Compare>::erase(const_value_type value)
{
  value_type return_value = NULL;
  iterator pos = find(value);
  if (storage_.end() != pos)
  {
    return_value = (*pos);
    storage_.erase(pos);
  }
  return return_value;
}

template <typename T, typename Compare>
int BaseSortedVector<T, Compare>::expand_ratio(const float expand_ratio)
{
  return storage_.expand_ratio(expand_ratio);
}

template <typename T, typename Compare>
bool BaseSortedVector<T, Compare>::need_expand(const float expand_ratio_ratio)
{
  return storage_.need_expand(expand_ratio_ratio);
}

template <typename T, typename Compare>
void BaseSortedVector<T, Compare>::clear()
{
  storage_.clear();
}
  
} // end namespace common
} // end namespace neptune

