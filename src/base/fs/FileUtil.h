#ifndef NEP_BASE_FS_FILEUTIL_H
#define NEP_BASE_FS_FILEUTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

namespace neptune {
namespace base {

#ifndef S_IRWXUGO
#define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)
#endif

class CFileUtil {
 public:
  static bool mkdirs(char *szDirPath);
  static bool isDirectory(const char *szDirPath);
  static bool isSymLink(const char *szDirPath);
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_FS_FILEUTIL_H
