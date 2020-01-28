#ifndef NEP_BASE_FS_DIRECTORYOP_H
#define NEP_BASE_FS_DIRECTORYOP_H

#include "base/common/Internal.h"

namespace neptune {
namespace base {

#ifndef S_IRWXUGO
#define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)
#endif

class DirectoryOp
{
 public:
  static bool exists(const char* filename);
  static bool delete_file(const char* filename);
  static bool is_directory(const char* dirname);
  static bool delete_directory(const char *dirname);
  static bool delete_directory_recursively(const char* directory, const bool delete_flag = false);
  static bool create_directory(const char* dirname, const mode_t dir_mode = 0);
  static bool create_full_path(const char* fullpath, const bool with_file = false, const mode_t dir_mode = 0);
  static bool rename(const char* srcfilename, const char* destfilename);
  static int64_t get_size(const char* filename);
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_FS_DIRECTORYOP_H
