#include "net.h"

namespace neptune {
namespace base {

HttpResponsePacket::HttpResponsePacket() {
  _status = true;
  _body = NULL;
  _bodyLen = 0;
  _isKeepAlive = false;
  _statusMessage = NULL;
}

HttpResponsePacket::~HttpResponsePacket() {
  if (_body) {
    ::free(_body);
  }
  if (_statusMessage) {
    ::free(_statusMessage);
    _statusMessage = NULL;
  }
}

bool HttpResponsePacket::encode(DataBuffer *output) {
  if (_statusMessage) {
    output->writeBytes(_statusMessage, strlen(_statusMessage));
    output->writeBytes("\r\n", 2);
  } else if (_status) { //HTTP/1.1 200 OK
    output->writeBytes(NET_HTTP_STATUS_OK, strlen(NET_HTTP_STATUS_OK));
  } else { // HTTP/1.1 404 Not Found
    output->writeBytes(NET_HTTP_STATUS_NOTFOUND, strlen(NET_HTTP_STATUS_NOTFOUND));
  }
  if (_isKeepAlive) {
    output->writeBytes(NET_HTTP_KEEP_ALIVE, strlen(NET_HTTP_KEEP_ALIVE));
  } else {
    output->writeBytes(NET_HTTP_CONN_CLOSE, strlen(NET_HTTP_CONN_CLOSE));
  }
  if (_headerMap.find("Content-Type") == _headerMap.end()) {
    output->writeBytes(NET_HTTP_CONTENT_TYPE, strlen(NET_HTTP_CONTENT_TYPE));
  }
  char tmp[64];
  int len = sprintf(tmp, NET_HTTP_CONTENT_LENGTH, _bodyLen);
  output->writeBytes(tmp, len);

  for (NET_STRING_MAP_ITER it=_headerMap.begin(); it!=_headerMap.end(); it++) {
    output->writeBytes(it->first.c_str(), strlen(it->first.c_str()));
    output->writeBytes(": ", 2);
    output->writeBytes(it->second.c_str(), strlen(it->second.c_str()));
    output->writeBytes("\r\n", 2);
  }

  output->writeBytes("\r\n", 2);
  // bodyLen
  output->writeBytes(_body, _bodyLen);
  //assert(_packetHeader._dataLen == output->getDataLen());

  return true;
}

bool HttpResponsePacket::decode(DataBuffer *input, PacketHeader *header) {
  return true;
}

void HttpResponsePacket::setHeader(const char *name, const char *value) {
  if (name[0] == 'C') {
    if (strcmp(name, "Connection") == 0 || strcmp(name, "Content-Length") == 0) {
      return;
    }
  }
  _headerMap[name] = value;
}

void HttpResponsePacket::setStatus(bool status, const char *statusMessage) {
  _status = status;
  if (_statusMessage) {
    ::free(_statusMessage);
    _statusMessage = NULL;
  }
  if (statusMessage) {
    _statusMessage = strdup(statusMessage);
  }
}

void HttpResponsePacket::setBody(const char *body, int len) {
  if (body) {
    _body = (char *) malloc(len);
    memcpy(_body, body, len);
    _bodyLen = len;
  }
}

void HttpResponsePacket::setKeepAlive(bool keepAlive) {
  _isKeepAlive = keepAlive;
}


}//namespace base
}//namespace neptune

