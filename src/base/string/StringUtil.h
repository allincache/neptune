#ifndef NEP_BASE_STRING_STRINGUTIL_H
#define NEP_BASE_STRING_STRINGUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <vector>

namespace neptune {
namespace base {

class CStringUtil {
 public:
  static int strToInt(const char *str, int d);
  static int isInt(const char *str);
  static char *strToLower(char *str);
  static char *strToUpper(char *str);
  static char *trim(char *str, const char *what = " ", int mode = 3);
  static int hashCode(const char *str);
  static int getPrimeHash(const char *str);
  static void split(char *str, const char *delim, std::vector<char*> &list);
  // urldecode
  static char *urlDecode(const char *src, char *dest);
  static unsigned int murMurHash(const void *key, int len);
  static std::string formatByteSize(double bytes);
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_STRING_STRINGUTIL_H
