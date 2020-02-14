#ifndef N_BASE_NET_HTTP_RESPONSE_PACKET_H
#define N_BASE_NET_HTTP_RESPONSE_PACKET_H

#include "base/network/simple/net.h"

namespace neptune {
namespace base {

struct str_hash {
  size_t operator()(const std::string& str) const {
    return __gnu_cxx::__stl_hash_string(str.c_str());
  }
};

typedef __gnu_cxx::hash_map<std::string, std::string, str_hash> NET_STRING_MAP;
typedef NET_STRING_MAP::iterator NET_STRING_MAP_ITER;

#define NET_HTTP_STATUS_OK "HTTP/1.1 200 OK\r\n"
#define NET_HTTP_STATUS_NOTFOUND "HTTP/1.1 404 Not Found\r\n"
#define NET_HTTP_KEEP_ALIVE "Connection: Keep-Alive\r\nKeep-Alive: timeout=10, max=10\r\n"
#define NET_HTTP_CONN_CLOSE "Connection: close\r\n"
#define NET_HTTP_CONTENT_TYPE "Content-Type: text/html\r\n"
#define NET_HTTP_CONTENT_LENGTH "Content-Length: %d\r\n"

class HttpResponsePacket : public Packet {
 public:
  HttpResponsePacket();

  ~HttpResponsePacket();

  void countDataLen();

  bool encode(DataBuffer *output);

  bool decode(DataBuffer *input, PacketHeader *header);

  void setHeader(const char *name, const char *value);

  void setStatus(bool status, const char *statusMessage = NULL);

  void setBody(const char *body, int len);

  void setKeepAlive(bool keepAlive);

 private:
  bool _status;                   
  char *_statusMessage;           
  char *_body;                    
  int _bodyLen;                   
  NET_STRING_MAP _headerMap;          
  bool _isKeepAlive;            
};

}//namespace base
}//namespace neptune

#endif

