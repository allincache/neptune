#ifndef NEP_BASE_TIME_TIME_H
#define NEP_BASE_TIME_TIME_H

#include <sys/time.h>
#include "base/include/Base.h"

namespace neptune {
namespace base {

class Time
{
 public:
  Time();

  enum Clock { Realtime, Monotonic };
  static Time now(Clock clock= Realtime);
  static Time seconds(int64_t usec);
  static Time milliSeconds(int64_t milli);
  static Time microSeconds(int64_t micro);
  operator timeval() const;

  int64_t toSeconds() const;
  int64_t toMilliSeconds() const;
  int64_t toMicroSeconds() const;

  double toSecondsDouble() const;
  double toMilliSecondsDouble() const;
  double toMicroSecondsDouble() const;

  std::string toDateTime() const;
  std::string toDuration() const;

  Time operator-() const
  {
    return Time(-_usec);
  }

  Time operator-(const Time& rhs) const
  {
    return Time(_usec - rhs._usec);
  }

  Time operator+(const Time& rhs) const
  {
    return Time(_usec + rhs._usec);
  }

  Time& operator+=(const Time& rhs)
  {
    _usec += rhs._usec;
    return *this;
  }

  Time& operator-=(const Time& rhs)
  {
    _usec -= rhs._usec;
    return *this;
  }

  bool operator<(const Time& rhs) const
  {
    return _usec < rhs._usec;
  }

  bool operator<=(const Time& rhs) const
  {
    return _usec <= rhs._usec;
  }

  bool operator>(const Time& rhs) const
  {
    return _usec > rhs._usec;
  }

  bool operator>=(const Time& rhs) const
  {
    return _usec >= rhs._usec;
  }

  bool operator==(const Time& rhs) const
  {
    return _usec == rhs._usec;
  }

  bool operator!=(const Time& rhs) const
  {
    return _usec != rhs._usec;
  }

  double operator/(const Time& rhs) const
  {
    return (double)_usec / (double)rhs._usec;
  }

  Time& operator*=(int rhs)
  {
    _usec *= rhs;
    return *this;
  }

  Time operator*(int rhs) const
  {
    Time t;
    t._usec = _usec * rhs;
    return t;
  }

  Time& operator/=(int rhs)
  {
    _usec /= rhs;
    return *this;
  }

  Time operator/(int rhs) const
  {
    Time t;
    t._usec = _usec / rhs;
    return t;
  }

  Time& operator*=(int64_t rhs)
  {
    _usec *= rhs;
    return *this;
  }

  Time operator*(int64_t rhs) const
  {
    Time t;
    t._usec = _usec * rhs;
    return t;
  }

  Time& operator/=(int64_t rhs)
  {
    _usec /= rhs;
    return *this;
  }

  Time operator/(int64_t rhs) const
  {
    Time t;
    t._usec = _usec / rhs;
    return t;
  }

  Time& operator*=(double rhs)
  {
    _usec = static_cast<int64_t>(static_cast<double>(_usec) * rhs);
    return *this;
  }

  Time operator*(double rhs) const
  {
    Time t;
    t._usec = static_cast<int64_t>(static_cast<double>(_usec) * rhs);
    return t;
  }

  Time& operator/=(double rhs)
  {
    _usec = static_cast<int64_t>(static_cast<double>(_usec) / rhs);
    return *this;
  }

  Time operator/(double rhs) const
  {
    Time t;
    t._usec = static_cast<int64_t>(static_cast<double>(_usec) / rhs);
    return t;
  }

  Time(int64_t);

 private:

  int64_t _usec;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_TIME_TIME_H
