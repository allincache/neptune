#include "Config.h"

using namespace std;

namespace neptune {
namespace base {
    
static CConfig _config;

CConfig::CConfig()
{
}

CConfig::~CConfig()
{
  for(auto it=m_configMap.begin(); 
    it!=m_configMap.end(); 
    ++it) {
    delete it->second;
  }
}

CConfig& CConfig::getCConfig()
{
  return _config;
}

int CConfig::parseValue(char *str, char *key, char *val)
{
  char *p, *p1, *name, *value;

  if (str == NULL)
    return -1;

  p = str;
  while ((*p) == ' ' || (*p) == '\t' || (*p) == '\r' || (*p) == '\n') p++;
  p1 = p + strlen(p);
  while(p1 > p) {
    p1 --;
    if (*p1 == ' ' || *p1 == '\t' || *p1 == '\r' || *p1 == '\n') continue;
    p1 ++;
    break;
  }
  (*p1) = '\0';
  if (*p == '#' || *p == '\0') return -1;
  p1 = strchr(str, '=');
  if (p1 == NULL) return -2;
  name = p;
  value = p1 + 1;
  while ((*(p1 - 1)) == ' ') p1--;
  (*p1) = '\0';

  while ((*value) == ' ') value++;
  p = strchr(value, '#');
  if (p == NULL) p = value + strlen(value);
  while ((*(p - 1)) <= ' ') p--;
  (*p) = '\0';
  if (name[0] == '\0')
    return -2;

  strcpy(key, name);
  strcpy(val, value);

  return 0;
}

char *CConfig::isSectionName(char *str) {
  if (str == NULL || strlen(str) <= 2 || (*str) != '[') 
    return NULL;
      
  char *p = str + strlen(str);
  while ((*(p-1)) == ' ' || (*(p-1)) == '\t' || (*(p-1)) == '\r' || (*(p-1)) == '\n') p--;
  if (*(p-1) != ']') return NULL;
  *(p-1) = '\0';
  
  p = str + 1;
  while(*p) {
    if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || (*p == '_')) {
    } else {
      return NULL;
    }
    p ++;
  }
  return (str+1);
}

int CConfig::load(const char *filename)
{
  FILE *fp;
  char key[128], value[4096], data[4096];
  int ret, line = 0;
  
  if ((fp = fopen(filename, "rb")) == NULL) {
    return EXIT_FAILURE;
  }
  
  STR_STR_MAP *m = NULL;
  while (fgets(data, 4096, fp)) {
    line ++;
    char *sName = isSectionName(data);
    if (sName != NULL) {
      auto it = m_configMap.find(sName);
      if (it == m_configMap.end()) {
        m = new STR_STR_MAP();
        m_configMap.insert(STR_MAP::value_type(/*CStringUtil::strToLower(sName)*/sName, m));
      } else {
        m = it->second;
      }
      continue;
    }
    ret = parseValue(data, key, value);
    if (ret == -2) {
      fclose(fp);
      return EXIT_FAILURE;
    }
    if (ret < 0) {
      continue;
    }
    if (m == NULL) {
      fclose(fp);
      return EXIT_FAILURE;
    }            
    //CStringUtil::strToLower(key);
    STR_STR_MAP_ITER it1 = m->find(key);
    if (it1 != m->end()) {
      it1->second += '\0';
      it1->second += value;
    } else {
      m->insert(STR_STR_MAP::value_type(key, value));
    }
  }
  fclose(fp);
  return EXIT_SUCCESS;
}

const char *CConfig::getString(const char *section, const string& key, const char *d)
{
  auto it = m_configMap.find(section);
  if (it == m_configMap.end()) {
    return d;
  }
  auto it1 = it->second->find(key);
  if (it1 == it->second->end()) {
    return d;
  }
  return it1->second.c_str();
}

vector<const char*> CConfig::getStringList(const char *section, const string& key) {
  vector<const char*> ret;
  auto it = m_configMap.find(section);
  if (it == m_configMap.end()) {
    return ret;
  }
  auto it1 = it->second->find(key);
  if (it1 == it->second->end()) {
    return ret;
  }
  const char *data = it1->second.data();
  const char *p = data;
  for(int i=0; i<(int)it1->second.size(); i++) {
    if (data[i] == '\0') {
      ret.push_back(p);
      p = data+i+1;
    }
  }
  ret.push_back(p);
  return ret;
}

int CConfig::getInt(const char *section, const string& key, int d)
{
  const char *str = getString(section, key);
  return CStringUtil::strToInt(str, d);
}

vector<int> CConfig::getIntList(const char *section, const string& key) {
  vector<int> ret;
  auto it = m_configMap.find(section);
  if (it == m_configMap.end()) {
    return ret;
  }
  auto it1 = it->second->find(key);
  if (it1 == it->second->end()) {
    return ret;
  }
  const char *data = it1->second.data();
  const char *p = data;
  for(int i=0; i<(int)it1->second.size(); i++) {
    if (data[i] == '\0') {
      ret.push_back(atoi(p));
      p = data+i+1;
    }
  }
  ret.push_back(atoi(p));
  return ret;
}

int CConfig::getSectionKey(const char *section, vector<string> &keys)
{
  auto it = m_configMap.find(section);
  if (it == m_configMap.end()) {
    return 0;
  }
  for(auto it1=it->second->begin(); it1!=it->second->end(); ++it1) {
    keys.push_back(it1->first);
  }
  return (int)keys.size();
}
            
int CConfig::getSectionName(vector<string> &sections)
{
  for(auto it=m_configMap.begin(); it!=m_configMap.end(); ++it)
  {
    sections.push_back(it->first);
  }
  return (int)sections.size();
}

string CConfig::toString()
{
  string result;
  for(auto it=m_configMap.begin(); it!=m_configMap.end(); ++it) {
    result += "[" + it->first + "]\n";
    for(auto it1=it->second->begin(); it1!=it->second->end(); ++it1) {
      string s = it1->second.c_str();
      result += "    " + it1->first + " = " + s + "\n";
      if (s.size() != it1->second.size()) {
        char *data = (char*)it1->second.data();
        char *p = NULL;
        for(int i=0; i<(int)it1->second.size(); i++) {
          if (data[i] != '\0') continue;
          if (p) result += "    " + it1->first + " = " + p + "\n";
          p = data+i+1;
        }
        if (p) result += "    " + it1->first + " = " + p + "\n";
      }
    }
  }
  result += "\n";    
  return result;
}

} //namespace base
} //namespace neptune

