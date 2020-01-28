#ifndef N_BASE_NET_DATA_BUFFER_H_
#define N_BASE_NET_DATA_BUFFER_H_

#define MAX_BUFFER_SIZE 2048

namespace neptune {
namespace base {

class DataBuffer {

 public:
  DataBuffer() {
    _pend = _pfree = _pdata = _pstart = NULL;
  }

  ~DataBuffer() {
    destroy();
  }

  void destroy() {
    if (_pstart) {
      free(_pstart);
      _pend = _pfree = _pdata = _pstart = NULL;
    }
  }

  char *getData() {
    return (char*)_pdata;
  }

  int getDataLen() {
    return static_cast<int32_t>(_pfree - _pdata);
  }

  char *getFree() {
    return (char*)_pfree;
  }

  int getFreeLen() {
    return static_cast<int32_t>(_pend - _pfree);
  }

  void drainData(int len) {
    _pdata += len;

    if (_pdata >= _pfree) {
      clear();
    }
  }

  void pourData(int len) {
    assert(_pend - _pfree >= len);
    _pfree += len;
  }

  void stripData(int len) {
    assert(_pfree - _pdata >= len);
    _pfree -= len;
  }

  void clear() {
    _pdata = _pfree = _pstart;
  }

  void shrink() {
    if (_pstart == NULL) {
      return;
    }
    if ((_pend - _pstart) <= MAX_BUFFER_SIZE || (_pfree - _pdata) > MAX_BUFFER_SIZE) {
      return;
    }

    int dlen = static_cast<int32_t>(_pfree - _pdata);
    if (dlen < 0) dlen = 0;

    unsigned char *newbuf = (unsigned char*)malloc(MAX_BUFFER_SIZE);
    assert(newbuf != NULL);

    if (dlen > 0) {
      memcpy(newbuf, _pdata, dlen);
    }
    free(_pstart);

    _pdata = _pstart = newbuf;
    _pfree = _pstart + dlen;
    _pend = _pstart + MAX_BUFFER_SIZE;

    return;
  }


  void writeInt8(uint8_t n) {
    expand(1);
    *_pfree++ = (unsigned char)n;
  }

  void writeInt16(uint16_t n) {
    expand(2);
    _pfree[1] = (unsigned char)n;
    n = static_cast<uint16_t>(n >> 8);
    _pfree[0] = (unsigned char)n;
    _pfree += 2;
  }

  void writeInt32(uint32_t n) {
    expand(4);
    _pfree[3] = (unsigned char)n;
    n >>= 8;
    _pfree[2] = (unsigned char)n;
    n >>= 8;
    _pfree[1] = (unsigned char)n;
    n >>= 8;
    _pfree[0] = (unsigned char)n;
    _pfree += 4;
  }

  void writeInt64(uint64_t n) {
    expand(8);
    _pfree[7] = (unsigned char)n;
    n >>= 8;
    _pfree[6] = (unsigned char)n;
    n >>= 8;
    _pfree[5] = (unsigned char)n;
    n >>= 8;
    _pfree[4] = (unsigned char)n;
    n >>= 8;
    _pfree[3] = (unsigned char)n;
    n >>= 8;
    _pfree[2] = (unsigned char)n;
    n >>= 8;
    _pfree[1] = (unsigned char)n;
    n >>= 8;
    _pfree[0] = (unsigned char)n;
    _pfree += 8;
  }

  void writeBytes(const void *src, int len) {
    expand(len);
    memcpy(_pfree, src, len);
    _pfree += len;
  }

  void fillInt8(unsigned char *dst, uint8_t n) {
    *dst = n;
  }

  void fillInt16(unsigned char *dst, uint16_t n) {
    dst[1] = (unsigned char)n;
    n = static_cast<uint16_t>(n >> 8);
    dst[0] = (unsigned char)n;
  }

  void fillInt32(unsigned char *dst, uint32_t n) {
    dst[3] = (unsigned char)n;
    n >>= 8;
    dst[2] = (unsigned char)n;
    n >>= 8;
    dst[1] = (unsigned char)n;
    n >>= 8;
    dst[0] = (unsigned char)n;
  }
  
  void fillInt64(unsigned char *dst, uint64_t n) {
    dst[7] = (unsigned char)n;
    n >>= 8;
    dst[6] = (unsigned char)n;
    n >>= 8;
    dst[5] = (unsigned char)n;
    n >>= 8;
    dst[4] = (unsigned char)n;
    n >>= 8;
    dst[3] = (unsigned char)n;
    n >>= 8;
    dst[2] = (unsigned char)n;
    n >>= 8;
    dst[1] = (unsigned char)n;
    n >>= 8;
    dst[0] = (unsigned char)n;
  }

  void writeString(const char *str) {
    int len = (str ? static_cast<int32_t>(strlen(str)) : 0);
    if (len>0) len ++;
    expand(static_cast<int32_t>(len+sizeof(uint32_t)));
    writeInt32(len);
    if (len>0) {
      memcpy(_pfree, str, len);
      _pfree += (len);
    }
  }

  void writeString(const std::string& str) {
    writeString(str.c_str());
  }

  void writeVector(const std::vector<int32_t>& v) {
    const uint32_t iLen = static_cast<uint32_t>(v.size());
    writeInt32(iLen);
    for (uint32_t i = 0; i < iLen; ++i) {
      writeInt32(v[i]); 
    }
  }

  void writeVector(const std::vector<uint32_t>& v) {
    const uint32_t iLen = static_cast<uint32_t>(v.size());
    writeInt32(iLen);
    for (uint32_t i = 0; i < iLen; ++i) {
      writeInt32(v[i]);
    } 
  }

  void writeVector(const std::vector<int64_t>& v) {
    const uint32_t iLen = static_cast<uint32_t>(v.size());
    writeInt32(iLen);
    for (uint32_t i = 0; i < iLen; ++i) {
      writeInt64(v[i]);
    }
  }

  void writeVector(const std::vector<uint64_t>& v) {
    const uint32_t iLen = static_cast<uint32_t>(v.size());
    writeInt32(iLen);
    for (uint32_t i = 0; i < iLen; ++i) {
      writeInt64(v[i]);
    }
  }

  uint8_t readInt8() {
    return (*_pdata++);
  }

  uint16_t readInt16() {
    uint16_t n = _pdata[0];
    n = static_cast<uint16_t>(n << 8);
    n = static_cast<uint16_t>(n | _pdata[1]);
    _pdata += 2;
    return n;
  }

  uint32_t readInt32() {
    uint32_t n = _pdata[0];
    n <<= 8;
    n |= _pdata[1];
    n <<= 8;
    n |= _pdata[2];
    n <<= 8;
    n |= _pdata[3];
    _pdata += 4;
    assert(_pfree>=_pdata);
    return n;
  }

  uint64_t readInt64() {
    uint64_t n = _pdata[0];
    n <<= 8;
    n |= _pdata[1];
    n <<= 8;
    n |= _pdata[2];
    n <<= 8;
    n |= _pdata[3];
    n <<= 8;
    n |= _pdata[4];
    n <<= 8;
    n |= _pdata[5];
    n <<= 8;
    n |= _pdata[6];
    n <<= 8;
    n |= _pdata[7];
    _pdata += 8;
    assert(_pfree>=_pdata);
    return n;
  }

  bool readBytes(void *dst, int len) {
    if (_pdata + len > _pfree) {
      return false;
    }
    memcpy(dst, _pdata, len);
    _pdata += len;
    assert(_pfree>=_pdata);
    return true;
  }

  bool readString(char *&str, int len) {
    if (_pdata + sizeof(int) > _pfree) {
      return false;
    }
    int slen = readInt32();
    if (_pfree - _pdata < slen) {
      slen = static_cast<int32_t>(_pfree - _pdata);
    }
    if (str == NULL && slen > 0) {
      str = (char*)malloc(slen);
      len = slen;
    }
    if (len > slen) {
      len = slen;
    }
    if (len > 0) {
      memcpy(str, _pdata, len);
      str[len-1] = '\0';
    }
    _pdata += slen;
    assert(_pfree>=_pdata);
    return true;
  }

  bool readVector(std::vector<int32_t>& v) {
    const uint32_t len = readInt32();
    for (uint32_t i = 0; i < len; ++i) {
      v.push_back(readInt32());
    }
    return true; 
  }

  bool readVector(std::vector<uint32_t>& v) {
    const uint32_t len = readInt32();
    for (uint32_t i = 0; i < len; ++i) {
      v.push_back(readInt32());
    }
    return true; 
  }

  bool readVector(std::vector<int64_t>& v) {
    const uint32_t len = readInt32();
    for (uint32_t i = 0; i < len; ++i) {
      v.push_back(readInt64());
    }
    return true; 
  }

  bool readVector(std::vector<uint64_t>& v) {
    const uint32_t len = readInt32();
    for (uint32_t i = 0; i < len; ++i) {
      v.push_back(readInt64());
    }
    return true; 
  }

  void ensureFree(int len) {
    expand(len);
  }

  int findBytes(const char *findstr, int len) {
    int dLen = static_cast<int32_t>(_pfree - _pdata - len + 1);
    for (int i=0; i<dLen; i++) {
      if (_pdata[i] == findstr[0] && memcmp(_pdata+i, findstr, len) == 0) {
        return i;
      }
    }
    return -1;
  }

 private:
  /*
    * expand
    */
  inline void expand(int need) {
    if (_pstart == NULL) {
      int len = 256;
      while (len < need) len <<= 1;
      _pfree = _pdata = _pstart = (unsigned char*)malloc(len);
      _pend = _pstart + len;
    } else if (_pend - _pfree < need) { 
      int flen = static_cast<int32_t>((_pend - _pfree) + (_pdata - _pstart));
      int dlen = static_cast<int32_t>(_pfree - _pdata);

      if (flen < need || flen * 4 < dlen) {
        int bufsize = static_cast<int32_t>((_pend - _pstart) * 2);
        while (bufsize - dlen < need)
            bufsize <<= 1;

        unsigned char *newbuf = (unsigned char *)malloc(bufsize);
        if (newbuf == NULL)
        {
        //  LOG(ERROR, "expand data buffer failed, length: %d", bufsize);
        }
        assert(newbuf != NULL);
        if (dlen > 0) {
            memcpy(newbuf, _pdata, dlen);
        }
        free(_pstart);

        _pdata = _pstart = newbuf;
        _pfree = _pstart + dlen;
        _pend = _pstart + bufsize;
      } else {
        memmove(_pstart, _pdata, dlen);
        _pfree = _pstart + dlen;
        _pdata = _pstart;
      }
    }
  }

 private:
  unsigned char *_pstart;     
  unsigned char *_pend;        
  unsigned char *_pfree;        
  unsigned char *_pdata;        
};

} //namespace base
} //namespace neptune

#endif
