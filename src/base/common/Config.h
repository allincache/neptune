#ifndef N_BASE_COMMON_CONFIG_H
#define N_BASE_COMMON_CONFIG_H

#include <string>
#include <ext/hash_map>
#include "base/string/StringUtil.h"
#include "base/log/Log.h"

namespace neptune {
namespace base {

struct config_str_hash {
  size_t operator()(const std::string& str) const {
    return __gnu_cxx::__stl_hash_string(str.c_str());
  }
};

struct char_equal {
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) == 0;
  }
};

typedef __gnu_cxx::hash_map<std::string, std::string, config_str_hash> STR_STR_MAP;
typedef STR_STR_MAP::iterator STR_STR_MAP_ITER;
typedef __gnu_cxx::hash_map<std::string, STR_STR_MAP*, config_str_hash> STR_MAP;
typedef STR_MAP::iterator STR_MAP_ITER;

#define NEP_CONFIG neptune::base::CConfig::getCConfig()

class CConfig {
 public:
  CConfig();
  ~CConfig();
  
  int load(const char *filename);
  const char *getString(const char *section, const std::string& key, const char *d = NULL);
  std::vector<const char*> getStringList(const char *section, const std::string& key);
  int getInt(char const *section, const std::string& key, int d = 0);
  std::vector<int> getIntList(const char *section, const std::string& key);
  int getSectionKey(const char *section, std::vector<std::string> &keys);
  int getSectionName(std::vector<std::string> &sections);
  std::string toString();
  static CConfig& getCConfig();
  
 private:
  STR_MAP m_configMap;
  
 private:
  int parseValue(char *str, char *key, char *val);
  char *isSectionName(char *str);     
};

} //namespace base
} //namespace neptune

#endif //N_BASE_COMMON_CONFIG_H
