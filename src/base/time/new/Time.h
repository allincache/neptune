#ifndef BASE_TIME_TIME_H_
#define BASE_TIME_TIME_H_

#include "base/thread/new/PosixThread.h"

#include <stdint.h>
#include <time.h>
#include <iosfwd>
#include <limits>
#include <unistd.h>
#include <sys/time.h>

namespace base {

class PosixThreadHandle;
class TimeDelta;

namespace time_internal {

// Add or subtract |value| from a TimeDelta. The int64_t argument and return
// value are in terms of a microsecond timebase.
int64_t SaturatedAdd(TimeDelta delta, int64_t value);
int64_t SaturatedSub(TimeDelta delta, int64_t value);

}  // namespace time_internal

// TimeDelta

class TimeDelta {
 public:
  constexpr TimeDelta() : delta_(0) {}

  // Converts units of time to TimeDeltas.
  static constexpr TimeDelta FromDays(int days);
  static constexpr TimeDelta FromHours(int hours);
  static constexpr TimeDelta FromMinutes(int minutes);
  static constexpr TimeDelta FromSeconds(int64_t secs);
  static constexpr TimeDelta FromMilliseconds(int64_t ms);
  static constexpr TimeDelta FromMicroseconds(int64_t us);
  static constexpr TimeDelta FromNanoseconds(int64_t ns);
  static constexpr TimeDelta FromSecondsD(double secs);
  static constexpr TimeDelta FromMillisecondsD(double ms);
  static constexpr TimeDelta FromMicrosecondsD(double us);
  static constexpr TimeDelta FromNanosecondsD(double ns);
  static TimeDelta FromTimeSpec(const timespec& ts);

  static constexpr TimeDelta FromInternalValue(int64_t delta) {
    return TimeDelta(delta);
  }

  static constexpr TimeDelta Max();

  static constexpr TimeDelta Min();

  constexpr int64_t ToInternalValue() const { return delta_; }

  constexpr bool is_zero() const { return delta_ == 0; }

  constexpr bool is_max() const {
    return delta_ == std::numeric_limits<int64_t>::max();
  }

  constexpr bool is_min() const {
    return delta_ == std::numeric_limits<int64_t>::min();
  }

  struct timespec ToTimeSpec() const;

  int InDays() const;
  int InDaysFloored() const;
  int InHours() const;
  int InMinutes() const;
  double InSecondsF() const;
  int64_t InSeconds() const;
  double InMillisecondsF() const;
  int64_t InMilliseconds() const;
  int64_t InMillisecondsRoundedUp() const;
  constexpr int64_t InMicroseconds() const { return delta_; }
  double InMicrosecondsF() const;
  int64_t InNanoseconds() const;

  TimeDelta operator+(TimeDelta other) const {
    return TimeDelta(time_internal::SaturatedAdd(*this, other.delta_));
  }

  TimeDelta operator-(TimeDelta other) const {
    return TimeDelta(time_internal::SaturatedSub(*this, other.delta_));
  }

  TimeDelta& operator+=(TimeDelta other) {
    return *this = (*this + other);
  }

  TimeDelta& operator-=(TimeDelta other) {
    return *this = (*this - other);
  }

  constexpr TimeDelta operator-() const { return TimeDelta(-delta_); }

  template <typename T>
  TimeDelta operator*(T a) const {
    int64_t rv(delta_);
    rv *= a;
    // if (rv.IsValid())
    //  return TimeDelta(rv.ValueOrDie());
    // Matched sign overflows. Mismatched sign underflows.
    if ((delta_ < 0) ^ (a < 0))
      return TimeDelta(std::numeric_limits<int64_t>::min());
    return TimeDelta(std::numeric_limits<int64_t>::max());
  }

  template <typename T>
  constexpr TimeDelta operator/(T a) const {
    int64_t rv(delta_);
    rv /= a;
    //if (rv.IsValid())
    //  return TimeDelta(rv.ValueOrDie());
    // Matched sign overflows. Mismatched sign underflows.
    // Special case to catch divide by zero.
    if ((delta_ < 0) ^ (a <= 0))
      return TimeDelta(std::numeric_limits<int64_t>::min());
    return TimeDelta(std::numeric_limits<int64_t>::max());
  }

  template <typename T>
  TimeDelta& operator*=(T a) {
    return *this = (*this * a);
  }

  template <typename T>
  constexpr TimeDelta& operator/=(T a) {
    return *this = (*this / a);
  }

  constexpr int64_t operator/(TimeDelta a) const { return delta_ / a.delta_; }

  constexpr TimeDelta operator%(TimeDelta a) const {
    return TimeDelta(delta_ % a.delta_);
  }

  // Comparison operators.
  constexpr bool operator==(TimeDelta other) const {
    return delta_ == other.delta_;
  }
  constexpr bool operator!=(TimeDelta other) const {
    return delta_ != other.delta_;
  }
  constexpr bool operator<(TimeDelta other) const {
    return delta_ < other.delta_;
  }
  constexpr bool operator<=(TimeDelta other) const {
    return delta_ <= other.delta_;
  }
  constexpr bool operator>(TimeDelta other) const {
    return delta_ > other.delta_;
  }
  constexpr bool operator>=(TimeDelta other) const {
    return delta_ >= other.delta_;
  }

 private:
  friend int64_t time_internal::SaturatedAdd(TimeDelta delta, int64_t value);
  friend int64_t time_internal::SaturatedSub(TimeDelta delta, int64_t value);
  constexpr explicit TimeDelta(int64_t delta_us) : delta_(delta_us) {}
  static constexpr TimeDelta FromDouble(double value);
  static constexpr TimeDelta FromProduct(int64_t value, int64_t positive_value);
  // Delta in microseconds.
  int64_t delta_;
};

template <typename T>
TimeDelta operator*(T a, TimeDelta td) {
  return td * a;
}

// For logging use only.
std::ostream& operator<<(std::ostream& os, TimeDelta time_delta);


namespace time_internal {

// TimeBase
template<class TimeClass>
class TimeBase {
 public:
  static constexpr int64_t kHoursPerDay = 24;
  static constexpr int64_t kSecondsPerMinute = 60;
  static constexpr int64_t kSecondsPerHour = 60 * kSecondsPerMinute;
  static constexpr int64_t kMillisecondsPerSecond = 1000;
  static constexpr int64_t kMillisecondsPerDay =
      kMillisecondsPerSecond * 60 * 60 * kHoursPerDay;
  static constexpr int64_t kMicrosecondsPerMillisecond = 1000;
  static constexpr int64_t kMicrosecondsPerSecond =
      kMicrosecondsPerMillisecond * kMillisecondsPerSecond;
  static constexpr int64_t kMicrosecondsPerMinute = kMicrosecondsPerSecond * 60;
  static constexpr int64_t kMicrosecondsPerHour = kMicrosecondsPerMinute * 60;
  static constexpr int64_t kMicrosecondsPerDay =
      kMicrosecondsPerHour * kHoursPerDay;
  static constexpr int64_t kMicrosecondsPerWeek = kMicrosecondsPerDay * 7;
  static constexpr int64_t kNanosecondsPerMicrosecond = 1000;
  static constexpr int64_t kNanosecondsPerSecond =
      kNanosecondsPerMicrosecond * kMicrosecondsPerSecond;

  // Returns true if this object has not been initialized.
  //
  // Warning: Be careful when writing code that performs math on time values,
  // since it's possible to produce a valid "zero" result that should not be
  // interpreted as a "null" value.
  constexpr bool is_null() const { return us_ == 0; }

  // Returns true if this object represents the maximum/minimum time.
  constexpr bool is_max() const {
    return us_ == std::numeric_limits<int64_t>::max();
  }

  constexpr bool is_min() const {
    return us_ == std::numeric_limits<int64_t>::min();
  }

  // Returns the maximum/minimum times, which should be greater/less than than
  // any reasonable time with which we might compare it.
  static TimeClass Max() {
    return TimeClass(std::numeric_limits<int64_t>::max());
  }

  static TimeClass Min() {
    return TimeClass(std::numeric_limits<int64_t>::min());
  }

  constexpr int64_t ToInternalValue() const { return us_; }

  constexpr TimeDelta since_origin() const {
    return TimeDelta::FromMicroseconds(us_);
  }

  TimeClass& operator=(TimeClass other) {
    us_ = other.us_;
    return *(static_cast<TimeClass*>(this));
  }

  // Compute the difference between two times.
  TimeDelta operator-(TimeClass other) const {
    return TimeDelta::FromMicroseconds(us_ - other.us_);
  }

  // Return a new time modified by some delta.
  TimeClass operator+(TimeDelta delta) const {
    return TimeClass(time_internal::SaturatedAdd(delta, us_));
  }
  TimeClass operator-(TimeDelta delta) const {
    return TimeClass(-time_internal::SaturatedSub(delta, us_));
  }

  // Modify by some time delta.
  TimeClass& operator+=(TimeDelta delta) {
    return static_cast<TimeClass&>(*this = (*this + delta));
  }
  TimeClass& operator-=(TimeDelta delta) {
    return static_cast<TimeClass&>(*this = (*this - delta));
  }

  // Comparison operators
  bool operator==(TimeClass other) const {
    return us_ == other.us_;
  }
  bool operator!=(TimeClass other) const {
    return us_ != other.us_;
  }
  bool operator<(TimeClass other) const {
    return us_ < other.us_;
  }
  bool operator<=(TimeClass other) const {
    return us_ <= other.us_;
  }
  bool operator>(TimeClass other) const {
    return us_ > other.us_;
  }
  bool operator>=(TimeClass other) const {
    return us_ >= other.us_;
  }

 protected:
  constexpr explicit TimeBase(int64_t us) : us_(us) {}

  // Time value in a microsecond timebase.
  int64_t us_;
};

}  // namespace time_internal

template<class TimeClass>
inline TimeClass operator+(TimeDelta delta, TimeClass t) {
  return t + delta;
}


// Represents a wall clock time in UTC. Values are not guaranteed to be
// monotonically non-decreasing and are subject to large amounts of skew.
class Time : public time_internal::TimeBase<Time> {
 public:
  // Offset of UNIX epoch (1970-01-01 00:00:00 UTC) from Windows FILETIME epoch
  // (1601-01-01 00:00:00 UTC), in microseconds. This value is derived from the
  // following: ((1970-1601)*365+89)*24*60*60*1000*1000, where 89 is the number
  // of leap year days between 1601 and 1970: (1970-1601)/4 excluding 1700,
  // 1800, and 1900.
  static constexpr int64_t kTimeTToMicrosecondsOffset =
      INT64_C(11644473600000000);

// kExplodedMinYear and kExplodedMaxYear define the platform-specific limits
// for values passed to FromUTCExploded() and FromLocalExploded(). Those
// functions will return false if passed values outside these limits. The limits
// are inclusive, meaning that the API should support all dates within a given
// limit year.
  static constexpr int kExplodedMinYear =
      (sizeof(time_t) == 4 ? 1902 : std::numeric_limits<int>::min());
  static constexpr int kExplodedMaxYear =
      (sizeof(time_t) == 4 ? 2037 : std::numeric_limits<int>::max());

  // Represents an exploded time that can be formatted nicely. This is kind of
  // like the Win32 SYSTEMTIME structure or the Unix "struct tm" with a few
  // additions and changes to prevent errors.
  struct Exploded {
    int year;          // Four digit year "2007"
    int month;         // 1-based month (values 1 = January, etc.)
    int day_of_week;   // 0-based day of week (0 = Sunday, etc.)
    int day_of_month;  // 1-based day of month (1-31)
    int hour;          // Hour within the current day (0-23)
    int minute;        // Minute within the current hour (0-59)
    int second;        // Second within the current minute (0-59 plus leap
                       //   seconds which may take it up to 60).
    int millisecond;   // Milliseconds within the current second (0-999)

    // A cursory test for whether the data members are within their
    // respective ranges. A 'true' return value does not guarantee the
    // Exploded value can be successfully converted to a Time value.
    bool HasValidValues() const;
  };

  // Contains the NULL time. Use Time::Now() to get the current time.
  constexpr Time() : TimeBase(0) {}

  // Returns the time for epoch in Unix-like system (Jan 1, 1970).
  static Time UnixEpoch();

  // Returns the current time. Watch out, the system might adjust its clock
  // in which case time will actually go backwards. We don't guarantee that
  // times are increasing, or that two calls to Now() won't be the same.
  static Time Now();

  // Returns the current time. Same as Now() except that this function always
  // uses system time so that there are no discrepancies between the returned
  // time and system time even on virtual environments including our test bot.
  // For timing sensitive unittests, this function should be used.
  static Time NowFromSystemTime();

  // Converts to/from TimeDeltas relative to the Windows epoch (1601-01-01
  // 00:00:00 UTC). Prefer these methods for opaque serialization and
  // deserialization of time values, e.g.
  //
  //   // Serialization:
  //   base::Time last_updated = ...;
  //   SaveToDatabase(last_updated.ToDeltaSinceWindowsEpoch().InMicroseconds());
  //
  //   // Deserialization:
  //   base::Time last_updated = base::Time::FromDeltaSinceWindowsEpoch(
  //       base::TimeDelta::FromMicroseconds(LoadFromDatabase()));
  static Time FromDeltaSinceWindowsEpoch(TimeDelta delta);
  TimeDelta ToDeltaSinceWindowsEpoch() const;

  // Converts to/from time_t in UTC and a Time class.
  static Time FromTimeT(time_t tt);
  time_t ToTimeT() const;

  // Converts time to/from a double which is the number of seconds since epoch
  // (Jan 1, 1970).  Webkit uses this format to represent time.
  // Because WebKit initializes double time value to 0 to indicate "not
  // initialized", we map it to empty Time object that also means "not
  // initialized".
  static Time FromDoubleT(double dt);
  double ToDoubleT() const;

  // Converts the timespec structure to time. MacOS X 10.8.3 (and tentatively,
  // earlier versions) will have the |ts|'s tv_nsec component zeroed out,
  // having a 1 second resolution, which agrees with
  // https://developer.apple.com/legacy/library/#technotes/tn/tn1150.html#HFSPlusDates.
  static Time FromTimeSpec(const timespec& ts);

  static Time FromTimeVal(struct timeval t);
  struct timeval ToTimeVal() const;

  static constexpr Time FromInternalValue(int64_t us) { return Time(us); }

 private:
  friend class time_internal::TimeBase<Time>;
  constexpr explicit Time(int64_t us) : TimeBase(us) {}
};

// static
constexpr TimeDelta TimeDelta::FromDays(int days) {
  return days == std::numeric_limits<int>::max()
             ? Max()
             : TimeDelta(days * Time::kMicrosecondsPerDay);
}

// static
constexpr TimeDelta TimeDelta::FromHours(int hours) {
  return hours == std::numeric_limits<int>::max()
             ? Max()
             : TimeDelta(hours * Time::kMicrosecondsPerHour);
}

// static
constexpr TimeDelta TimeDelta::FromMinutes(int minutes) {
  return minutes == std::numeric_limits<int>::max()
             ? Max()
             : TimeDelta(minutes * Time::kMicrosecondsPerMinute);
}

// static
constexpr TimeDelta TimeDelta::FromSeconds(int64_t secs) {
  return FromProduct(secs, Time::kMicrosecondsPerSecond);
}

// static
constexpr TimeDelta TimeDelta::FromMilliseconds(int64_t ms) {
  return FromProduct(ms, Time::kMicrosecondsPerMillisecond);
}

// static
constexpr TimeDelta TimeDelta::FromMicroseconds(int64_t us) {
  return TimeDelta(us);
}

// static
constexpr TimeDelta TimeDelta::FromNanoseconds(int64_t ns) {
  return TimeDelta(ns / Time::kNanosecondsPerMicrosecond);
}

// static
constexpr TimeDelta TimeDelta::FromSecondsD(double secs) {
  return FromDouble(secs * Time::kMicrosecondsPerSecond);
}

// static
constexpr TimeDelta TimeDelta::FromMillisecondsD(double ms) {
  return FromDouble(ms * Time::kMicrosecondsPerMillisecond);
}

// static
constexpr TimeDelta TimeDelta::FromMicrosecondsD(double us) {
  return FromDouble(us);
}

// static
constexpr TimeDelta TimeDelta::FromNanosecondsD(double ns) {
  return FromDouble(ns / Time::kNanosecondsPerMicrosecond);
}

// static
constexpr TimeDelta TimeDelta::Max() {
  return TimeDelta(std::numeric_limits<int64_t>::max());
}

// static
constexpr TimeDelta TimeDelta::Min() {
  return TimeDelta(std::numeric_limits<int64_t>::min());
}

// static
constexpr TimeDelta TimeDelta::FromDouble(double value) {
  // TODO(crbug.com/612601): Use saturated_cast<int64_t>(value) once we sort out
  // the Min() behavior.
  return value > std::numeric_limits<int64_t>::max()
             ? Max()
             : value < std::numeric_limits<int64_t>::min()
                   ? Min()
                   : TimeDelta(static_cast<int64_t>(value));
}

// static
constexpr TimeDelta TimeDelta::FromProduct(int64_t value,
                                           int64_t positive_value) {
  //DCHECK(positive_value > 0);  // NOLINT, DCHECK_GT isn't constexpr.
  return value > std::numeric_limits<int64_t>::max() / positive_value
             ? Max()
             : value < std::numeric_limits<int64_t>::min() / positive_value
                   ? Min()
                   : TimeDelta(value * positive_value);
}

// For logging use only.
std::ostream& operator<<(std::ostream& os, Time time);

// TimeTicks
// Represents monotonically non-decreasing clock time.
class TimeTicks : public time_internal::TimeBase<TimeTicks> {
 public:
  // The underlying clock used to generate new TimeTicks.
  enum class Clock {
    FUCHSIA_ZX_CLOCK_MONOTONIC,
    LINUX_CLOCK_MONOTONIC,
    IOS_CF_ABSOLUTE_TIME_MINUS_KERN_BOOTTIME,
    MAC_MACH_ABSOLUTE_TIME,
    WIN_QPC,
    WIN_ROLLOVER_PROTECTED_TIME_GET_TIME
  };

  constexpr TimeTicks() : TimeBase(0) {}

  // Platform-dependent tick count representing "right now." When
  // IsHighResolution() returns false, the resolution of the clock could be
  // as coarse as ~15.6ms. Otherwise, the resolution should be no worse than one
  // microsecond.
  static TimeTicks Now();

  // Returns true if the high resolution clock is working on this system and
  // Now() will return high resolution values. Note that, on systems where the
  // high resolution clock works but is deemed inefficient, the low resolution
  // clock will be used instead.
  static bool IsHighResolution() ;

  // Returns true if TimeTicks is consistent across processes, meaning that
  // timestamps taken on different processes can be safely compared with one
  // another. (Note that, even on platforms where this returns true, time values
  // from different threads that are within one tick of each other must be
  // considered to have an ambiguous ordering.)
  static bool IsConsistentAcrossProcesses() ;


  // Get an estimate of the TimeTick value at the time of the UnixEpoch. Because
  // Time and TimeTicks respond differently to user-set time and NTP
  // adjustments, this number is only an estimate. Nevertheless, this can be
  // useful when you need to relate the value of TimeTicks to a real time and
  // date. Note: Upon first invocation, this function takes a snapshot of the
  // realtime clock to establish a reference point.  This function will return
  // the same value for the duration of the application, but will be different
  // in future application runs.
  static TimeTicks UnixEpoch();

  // Returns |this| snapped to the next tick, given a |tick_phase| and
  // repeating |tick_interval| in both directions. |this| may be before,
  // after, or equal to the |tick_phase|.
  TimeTicks SnappedToNextTick(TimeTicks tick_phase,
                              TimeDelta tick_interval) const;

  // Returns an enum indicating the underlying clock being used to generate
  // TimeTicks timestamps. This function should only be used for debugging and
  // logging purposes.
  static Clock GetClock();

  // Converts an integer value representing TimeTicks to a class. This may be
  // used when deserializing a |TimeTicks| structure, using a value known to be
  // compatible. It is not provided as a constructor because the integer type
  // may be unclear from the perspective of a caller.
  //
  // DEPRECATED - Do not use in new code. For deserializing TimeTicks values,
  // prefer TimeTicks + TimeDelta(). http://crbug.com/634507
  static constexpr TimeTicks FromInternalValue(int64_t us) {
    return TimeTicks(us);
  }

 private:
  friend class time_internal::TimeBase<TimeTicks>;

  // Please use Now() to create a new object. This is for internal use
  // and testing.
  constexpr explicit TimeTicks(int64_t us) : TimeBase(us) {}
};

// For logging use only.
std::ostream& operator<<(std::ostream& os, TimeTicks time_ticks);


// Represents a clock, specific to a particular thread, than runs only while the
// thread is running.
class ThreadTicks : public time_internal::TimeBase<ThreadTicks> {
 public:
  constexpr ThreadTicks() : TimeBase(0) {}

  // Returns true if ThreadTicks::Now() is supported on this system.
  static bool IsSupported()  {
    return true;
  }

  // Waits until the initialization is completed. Needs to be guarded with a
  // call to IsSupported().
  static void WaitUntilInitialized() {
  }

  // Returns thread-specific CPU-time on systems that support this feature.
  // Needs to be guarded with a call to IsSupported(). Use this timer
  // to (approximately) measure how much time the calling thread spent doing
  // actual work vs. being de-scheduled. May return bogus results if the thread
  // migrates to another CPU between two calls. Returns an empty ThreadTicks
  // object until the initialization is completed. If a clock reading is
  // absolutely needed, call WaitUntilInitialized() before this method.
  static ThreadTicks Now();

  // Converts an integer value representing ThreadTicks to a class. This may be
  // used when deserializing a |ThreadTicks| structure, using a value known to
  // be compatible. It is not provided as a constructor because the integer type
  // may be unclear from the perspective of a caller.
  //
  // DEPRECATED - Do not use in new code. For deserializing ThreadTicks values,
  // prefer ThreadTicks + TimeDelta(). http://crbug.com/634507
  static constexpr ThreadTicks FromInternalValue(int64_t us) {
    return ThreadTicks(us);
  }

 private:
  friend class time_internal::TimeBase<ThreadTicks>;

  // Please use Now() or GetForThread() to create a new object. This is for
  // internal use and testing.
  constexpr explicit ThreadTicks(int64_t us) : TimeBase(us) {}

};

// For logging use only.
std::ostream& operator<<(std::ostream& os, ThreadTicks time_ticks);

}  // namespace base




namespace {

int64_t ConvertTimespecToMicros(const struct timespec& ts) {
  // On 32-bit systems, the calculation cannot overflow int64_t.
  // 2**32 * 1000000 + 2**64 / 1000 < 2**63
  if (sizeof(ts.tv_sec) <= 4 && sizeof(ts.tv_nsec) <= 8) {
    int64_t result = ts.tv_sec;
    result *= base::Time::kMicrosecondsPerSecond;
    result += (ts.tv_nsec / base::Time::kNanosecondsPerMicrosecond);
    return result;
  }
  int64_t result(ts.tv_sec);
  result *= base::Time::kMicrosecondsPerSecond;
  result += (ts.tv_nsec / base::Time::kNanosecondsPerMicrosecond);
  return result;
}

// Helper function to get results from clock_gettime() and convert to a
// microsecond timebase. Minimum requirement is MONOTONIC_CLOCK to be supported
// on the system. FreeBSD 6 has CLOCK_MONOTONIC but defines
// _POSIX_MONOTONIC_CLOCK to -1.
int64_t ClockNow(clockid_t clk_id) {
  struct timespec ts;
  //CHECK(clock_gettime(clk_id, &ts) == 0);
  return ConvertTimespecToMicros(ts);
}

}  // namespace

namespace base {

// Time -----------------------------------------------------------------------

namespace subtle {


Time TimeNowIgnoringOverride() {
  struct timeval tv;
  struct timezone tz = {0, 0};  // UTC
  //CHECK(gettimeofday(&tv, &tz) == 0);
  // Combine seconds and microseconds in a 64-bit field containing microseconds
  // since the epoch.  That's enough for nearly 600 centuries.  Adjust from
  // Unix (1970) to Windows (1601) epoch.
  return Time();
}

Time TimeNowFromSystemTimeIgnoringOverride() {
  // Just use TimeNowIgnoringOverride() because it returns the system time.
  return TimeNowIgnoringOverride();
}


}  // namespace subtle

// TimeTicks ------------------------------------------------------------------

namespace subtle {
TimeTicks TimeTicksNowIgnoringOverride() {
  //return TimeTicks() + TimeDelta::FromMicroseconds(ClockNow(CLOCK_MONOTONIC));
  return TimeTicks();
}
}  // namespace subtle

// static
TimeTicks::Clock TimeTicks::GetClock() {
  return Clock::LINUX_CLOCK_MONOTONIC;
}

// static
bool TimeTicks::IsHighResolution() {
  return true;
}

// static
bool TimeTicks::IsConsistentAcrossProcesses() {
  return true;
}

// ThreadTicks ----------------------------------------------------------------

namespace subtle {
ThreadTicks ThreadTicksNowIgnoringOverride() {
#if (defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME >= 0)) || \
    defined(OS_ANDROID)
  //return ThreadTicks() +
  //       TimeDelta::FromMicroseconds(ClockNow(CLOCK_THREAD_CPUTIME_ID));
  return ThreadTicks();
#else
  //NOTREACHED();
  return ThreadTicks();
#endif
}
}  // namespace subtle

}  // namespace base


#endif  // BASE_TIME_TIME_H_
