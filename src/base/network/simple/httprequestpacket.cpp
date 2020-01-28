#include "net.h"

namespace neptune {
namespace base {

HttpRequestPacket::HttpRequestPacket() {
  _strHeader = NULL;
  _strQuery = NULL;
  _isKeepAlive = false;
  _method = 0;
}

HttpRequestPacket::~HttpRequestPacket() {
  if (_strHeader) {
    ::free(_strHeader);
  }
}

bool HttpRequestPacket::encode(DataBuffer *output) {
  return true;
}

bool HttpRequestPacket::decode(DataBuffer *input, PacketHeader *header) {
  int len = header->_dataLen;
  _strHeader = (char*) malloc(len+1);
  input->readBytes(_strHeader, len);
  _strHeader[len] = '\0';
  int line = 0;
  int first = 1;

  char *p, *name = NULL, *value;
  p = value = _strHeader;
  while (*p) {
      if (*p == '\r' && *(p+1) == '\n') {
          if (value == p && line > 0) { 
              break;
          }
          *p = '\0';
          while (*value == ' ') value ++;
          if (line > 0) {
              if (strcmp(name, "Connection") == 0 && strcasecmp(value, "Keep-Alive") == 0) {
                  _isKeepAlive = true;
              } else {
                  _headerMap[name] = value;
              }
          } else {
              _strQuery = value;
          }
          value = p + 2;
          line ++;
          first = 1;
      } else if (line == 0 && *p == ' ') { 
          if (_method) {
              *p = '\0';
          } else if (strncmp(value, "GET ", 4) == 0) {    
              _method = 1;
              value = p + 1;
          }
      } else if (*p == ':' && first == 1) {
          *p = '\0';
          name = value;
          value = p + 1;
          first = 0;
      }
      p ++;
  }

  return true;
}

char *HttpRequestPacket::getQuery() {
  return _strQuery;
}

bool HttpRequestPacket::isKeepAlive() {
  return _isKeepAlive;
}

const char *HttpRequestPacket::findHeader(const char *name) {
  PSTR_MAP_ITER it = _headerMap.find(name);
  if (it != _headerMap.end()) {
    return it->second;
  }
  return NULL;
}

// Connection
Connection *HttpRequestPacket::getConnection() {
  return _connection;
}

// connection
void HttpRequestPacket::setConnection(Connection *connection) {
  _connection = connection;
}

}//namespace base
}//namespace neptune


