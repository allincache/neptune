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
  static Time seconds(Int64 usec);
  static Time milliSeconds(Int64 milli);
  static Time microSeconds(Int64 micro);
  operator timeval() const;

  Int64 toSeconds() const;
  Int64 toMilliSeconds() const;
  Int64 toMicroSeconds() const;

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

  Time& operator*=(Int64 rhs)
  {
      _usec *= rhs;
      return *this;
  }

  Time operator*(Int64 rhs) const
  {
      Time t;
      t._usec = _usec * rhs;
      return t;
  }

  Time& operator/=(Int64 rhs)
  {
      _usec /= rhs;
      return *this;
  }

  Time operator/(Int64 rhs) const
  {
      Time t;
      t._usec = _usec / rhs;
      return t;
  }

  Time& operator*=(double rhs)
  {
      _usec = static_cast<Int64>(static_cast<double>(_usec) * rhs);
      return *this;
  }

  Time operator*(double rhs) const
  {
      Time t;
      t._usec = static_cast<Int64>(static_cast<double>(_usec) * rhs);
      return t;
  }

  Time& operator/=(double rhs)
  {
      _usec = static_cast<Int64>(static_cast<double>(_usec) / rhs);
      return *this;
  }

  Time operator/(double rhs) const
  {
      Time t;
      t._usec = static_cast<Int64>(static_cast<double>(_usec) / rhs);
      return t;
  }

  Time(Int64);

 private:

  Int64 _usec;
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_TIME_TIME_H
