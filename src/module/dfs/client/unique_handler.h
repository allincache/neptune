#ifndef N_DFS_CLIENT_UNIQUEHANDLER_H_
#define N_DFS_CLIENT_UNIQUEHANDLER_H_

namespace neptune {
namespace dfs {

template<typename K, typename V>
class UniqueHandler
{
 public:
  UniqueHandler(){}
  virtual ~UniqueHandler(){}

  virtual int query(K& key, V& value) = 0;
  virtual int insert(K& key, V& value) = 0;
  virtual int32_t decrease(K& key, V& value, const int32_t count = 1) = 0;
  virtual int32_t increase(K& key, V& value, const int32_t count = 1) = 0;
  virtual int erase(K& key) = 0;
};


}
}

#endif
