#ifndef N_BASE_NET_HTTP_REQUEST_PACKET_H
#define N_BASE_NET_HTTP_REQUEST_PACKET_H

namespace neptune {
namespace base {

struct eqstr {
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) == 0;
  }
};

typedef __gnu_cxx::hash_map<const char*, const char*, __gnu_cxx::hash<const char*>, eqstr> PSTR_MAP;
typedef PSTR_MAP::iterator PSTR_MAP_ITER;

class HttpRequestPacket : public Packet {
 public:
  HttpRequestPacket();

  ~HttpRequestPacket();

  void countDataLen();
  bool encode(DataBuffer *output);

  bool decode(DataBuffer *input, PacketHeader *header);

  char *getQuery();

  bool isKeepAlive();

  const char *findHeader(const char *name);

  Connection *getConnection();

  void setConnection(Connection *connection);

private:
  char *_strHeader;       
  char *_strQuery;        
  bool _isKeepAlive;      
  int _method;            // get - 1
  PSTR_MAP _headerMap;    

  Connection *_connection;
};

}//namespace base
}//namespace neptune

#endif

