#ifndef N_DFS_DS_VERSION_H_
#define N_DFS_DS_VERSION_H_

namespace neptune {
namespace dfs {
namespace dataserver {

#if defined(__DATE__) && defined(__TIME__) && defined(PACKAGE) && defined(VERSION)
    static const char _build_description[] = "NEPTUNE File System(DFS), Version: " VERSION ", Build Time: " __DATE__ " " __TIME__;
#else
    static const char _build_description[] = "unknown";
#endif

class Version
{
 public:
  Version();
  static const char* get_build_description();
};

}
}
}

#endif //N_DFS_DS_VERSION_H_
