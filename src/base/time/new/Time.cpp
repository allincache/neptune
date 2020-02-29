#include "base/time/new/Time.h"

#include <cmath>
#include <ios>
#include <limits>
#include <ostream>
#include <sstream>

namespace base {

using TimeNowFunction = decltype(&Time::Now);
using TimeTicksNowFunction = decltype(&TimeTicks::Now);
using ThreadTicksNowFunction = decltype(&ThreadTicks::Now);

#define PR_TRUE 1
#define PR_FALSE 0

typedef enum { PR_FAILURE = -1, PR_SUCCESS = 0 } PRStatus;

namespace internal {

TimeNowFunction g_time_now_function = &subtle::TimeNowIgnoringOverride;

TimeNowFunction g_time_now_from_system_time_function =
    &subtle::TimeNowFromSystemTimeIgnoringOverride;

TimeTicksNowFunction g_time_ticks_now_function =
    &subtle::TimeTicksNowIgnoringOverride;

ThreadTicksNowFunction g_thread_ticks_now_function =
    &subtle::ThreadTicksNowIgnoringOverride;

}  // namespace internal

// TimeDelta ------------------------------------------------------------------

int TimeDelta::InDays() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int>::max();
  }
  return static_cast<int>(delta_ / Time::kMicrosecondsPerDay);
}

int TimeDelta::InDaysFloored() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int>::max();
  }
  int result = delta_ / Time::kMicrosecondsPerDay;
  int64_t remainder = delta_ - (result * Time::kMicrosecondsPerDay);
  if (remainder < 0) {
    --result;  // Use floor(), not trunc() rounding behavior.
  }
  return result;
}

int TimeDelta::InHours() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int>::max();
  }
  return static_cast<int>(delta_ / Time::kMicrosecondsPerHour);
}

int TimeDelta::InMinutes() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int>::max();
  }
  return static_cast<int>(delta_ / Time::kMicrosecondsPerMinute);
}

double TimeDelta::InSecondsF() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(delta_) / Time::kMicrosecondsPerSecond;
}

int64_t TimeDelta::InSeconds() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int64_t>::max();
  }
  return delta_ / Time::kMicrosecondsPerSecond;
}

double TimeDelta::InMillisecondsF() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(delta_) / Time::kMicrosecondsPerMillisecond;
}

int64_t TimeDelta::InMilliseconds() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int64_t>::max();
  }
  return delta_ / Time::kMicrosecondsPerMillisecond;
}

int64_t TimeDelta::InMillisecondsRoundedUp() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int64_t>::max();
  }
  int64_t result = delta_ / Time::kMicrosecondsPerMillisecond;
  int64_t remainder = delta_ - (result * Time::kMicrosecondsPerMillisecond);
  if (remainder > 0) {
    ++result;  // Use ceil(), not trunc() rounding behavior.
  }
  return result;
}

double TimeDelta::InMicrosecondsF() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(delta_);
}

int64_t TimeDelta::InNanoseconds() const {
  if (is_max()) {
    // Preserve max to prevent overflow.
    return std::numeric_limits<int64_t>::max();
  }
  return delta_ * Time::kNanosecondsPerMicrosecond;
}

namespace time_internal {

int64_t SaturatedAdd(TimeDelta delta, long value) {
  int64_t rv(delta.delta_);
  rv += value;
  //if (rv.IsValid())
  //  return rv.ValueOrDie();
  // Positive RHS overflows. Negative RHS underflows.
  if (value < 0)
    return std::numeric_limits<int64_t>::min();
  return std::numeric_limits<int64_t>::max();
}


int64_t SaturatedSub(TimeDelta delta, int64_t value) {
  int64_t rv(delta.delta_);
  rv -= value;
  //if (rv.IsValid())
  //  return rv.ValueOrDie();
  // Negative RHS overflows. Positive RHS underflows.
  if (value < 0)
    return std::numeric_limits<int64_t>::max();
  return std::numeric_limits<int64_t>::min();
}

}  // namespace time_internal

std::ostream& operator<<(std::ostream& os, TimeDelta time_delta) {
  return os << time_delta.InSecondsF() << " s";
}

// Time -----------------------------------------------------------------------

// static
Time Time::Now() {
  return internal::g_time_now_function();
}

// static
Time Time::NowFromSystemTime() {
  // Just use g_time_now_function because it returns the system time.
  return internal::g_time_now_from_system_time_function();
}

// static
Time Time::FromDeltaSinceWindowsEpoch(TimeDelta delta) {
  return Time(delta.InMicroseconds());
}

TimeDelta Time::ToDeltaSinceWindowsEpoch() const {
  return TimeDelta::FromMicroseconds(us_);
}

// static
Time Time::FromTimeT(time_t tt) {
  if (tt == 0)
    return Time();  // Preserve 0 so we can tell it doesn't exist.
  if (tt == std::numeric_limits<time_t>::max())
    return Max();
  return Time(kTimeTToMicrosecondsOffset) + TimeDelta::FromSeconds(tt);
}

time_t Time::ToTimeT() const {
  if (is_null())
    return 0;  // Preserve 0 so we can tell it doesn't exist.
  if (is_max()) {
    // Preserve max without offset to prevent overflow.
    return std::numeric_limits<time_t>::max();
  }
  if (std::numeric_limits<int64_t>::max() - kTimeTToMicrosecondsOffset <= us_) {
    //DLOG(WARNING) << "Overflow when converting base::Time with internal " <<
    //                 "value " << us_ << " to time_t.";
    return std::numeric_limits<time_t>::max();
  }
  return (us_ - kTimeTToMicrosecondsOffset) / kMicrosecondsPerSecond;
}

// static
Time Time::FromDoubleT(double dt) {
  if (dt == 0 || std::isnan(dt))
    return Time();  // Preserve 0 so we can tell it doesn't exist.
  return Time(kTimeTToMicrosecondsOffset) + TimeDelta::FromSecondsD(dt);
}

double Time::ToDoubleT() const {
  if (is_null())
    return 0;  // Preserve 0 so we can tell it doesn't exist.
  if (is_max()) {
    // Preserve max without offset to prevent overflow.
    return std::numeric_limits<double>::infinity();
  }
  return (static_cast<double>(us_ - kTimeTToMicrosecondsOffset) /
          static_cast<double>(kMicrosecondsPerSecond));
}

// static
Time Time::FromTimeSpec(const timespec& ts) {
  return FromDoubleT(ts.tv_sec +
                     static_cast<double>(ts.tv_nsec) /
                         base::Time::kNanosecondsPerSecond);
}

// static
Time Time::UnixEpoch() {
  Time time;
  time.us_ = kTimeTToMicrosecondsOffset;
  return time;
}

std::ostream& operator<<(std::ostream& os, Time time) {
  // Time::Exploded exploded;
  // time.UTCExplode(&exploded);
  // Use StringPrintf because iostreams formatting is painful.
  // return os << StringPrintf("%04d-%02d-%02d %02d:%02d:%02d.%03d UTC",
  //                           exploded.year,
  //                           exploded.month,
  //                           exploded.day_of_month,
  //                           exploded.hour,
  //                           exploded.minute,
  //                           exploded.second,
  //                           exploded.millisecond);
  return os << "test";
}

// TimeTicks ------------------------------------------------------------------

// static
TimeTicks TimeTicks::Now() {
  return internal::g_time_ticks_now_function();
}

TimeTicks TimeTicks::SnappedToNextTick(TimeTicks tick_phase,
                                       TimeDelta tick_interval) const {
  // |interval_offset| is the offset from |this| to the next multiple of
  // |tick_interval| after |tick_phase|, possibly negative if in the past.
  TimeDelta interval_offset = (tick_phase - *this) % tick_interval;
  // If |this| is exactly on the interval (i.e. offset==0), don't adjust.
  // Otherwise, if |tick_phase| was in the past, adjust forward to the next
  // tick after |this|.
  if (!interval_offset.is_zero() && tick_phase < *this)
    interval_offset += tick_interval;
  return *this + interval_offset;
}

std::ostream& operator<<(std::ostream& os, TimeTicks time_ticks) {
  // This function formats a TimeTicks object as "bogo-microseconds".
  // The origin and granularity of the count are platform-specific, and may very
  // from run to run. Although bogo-microseconds usually roughly correspond to
  // real microseconds, the only real guarantee is that the number never goes
  // down during a single run.
  const TimeDelta as_time_delta = time_ticks - TimeTicks();
  return os << as_time_delta.InMicroseconds() << " bogo-microseconds";
}

// ThreadTicks ----------------------------------------------------------------

// static
ThreadTicks ThreadTicks::Now() {
  return internal::g_thread_ticks_now_function();
}

std::ostream& operator<<(std::ostream& os, ThreadTicks thread_ticks) {
  const TimeDelta as_time_delta = thread_ticks - ThreadTicks();
  return os << as_time_delta.InMicroseconds() << " bogo-thread-microseconds";
}

// Time::Exploded -------------------------------------------------------------

inline bool is_in_range(int value, int lo, int hi) {
  return lo <= value && value <= hi;
}

bool Time::Exploded::HasValidValues() const {
  return is_in_range(month, 1, 12) &&
         is_in_range(day_of_week, 0, 6) &&
         is_in_range(day_of_month, 1, 31) &&
         is_in_range(hour, 0, 23) &&
         is_in_range(minute, 0, 59) &&
         is_in_range(second, 0, 60) &&
         is_in_range(millisecond, 0, 999);
}

}  // namespace base
