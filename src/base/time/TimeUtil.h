#ifndef NEP_BASE_TIME_TIMEUTIL_H
#define NEP_BASE_TIME_TIMEUTIL_H

#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

namespace neptune {
namespace base {

class CTimeUtil {
 public:
    static int64_t getTime();
    static int64_t getMonotonicTime();
    static char *timeToStr(time_t t, char *dest);
    static int strToTime(char *str);
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_TIME_TIMEUTIL_H
