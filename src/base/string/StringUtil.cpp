#include "StringUtil.h"

namespace neptune {
namespace base {

int CStringUtil::isInt(const char *p) {
  if (p == NULL || (*p) == '\0') 
    return 0;
  if ((*p) == '-') p ++;
  while((*p)) {
    if ((*p) < '0' || (*p) > '9') return 0;   
    p ++;
  }
  return 1;
}

int CStringUtil::strToInt(const char *str, int d)
{
  if (isInt(str)) {
    return atoi(str);
  } else {
    return d;
  }
}

char *CStringUtil::strToLower(char *pszBuf)
{
  if (pszBuf == NULL)
    return pszBuf;

  char *p = pszBuf;
  while (*p) {
    if ((*p) & 0x80)
      p++;
    else if ((*p) >= 'A' && (*p) <= 'Z')
      (*p) += 32;
    p++;
  }
  return pszBuf;
}

char *CStringUtil::strToUpper(char *pszBuf)
{
  if (pszBuf == NULL)
    return pszBuf;

  char *p = pszBuf;
  while (*p) {
    if ((*p) & 0x80)
      p++;
    else if ((*p) >= 'a' && (*p) <= 'z')
      (*p) -= 32;
    p++;
  }
  return pszBuf;
}

char *CStringUtil::trim(char *str, const char *what, int mode) 
{
  char mask[256];
  unsigned char *p;
  unsigned char *ret;
  memset(mask, 0, 256);
  ret = (unsigned char *)str;
  p = (unsigned char *)what;
  while(*p) {
    mask[*p] = '\1';
    p ++;
  }
  if (mode & 1) {
    p = ret; 
    while(*p) {
      if (!mask[*p]) {
        break;
      }
      ret ++;
      p ++;   
    }
  }
  if (mode & 2) {
    p = ret + strlen((const char*)ret) - 1;
    while(p>=ret) {
      if (!mask[*p]) {
        break;
      }
      p --;
    }
    p ++;
    *p = '\0';
  }
  return (char*)ret;
}

int CStringUtil::hashCode(const char *str)
{
  int h = 0;
  while(*str) {
    h = 31*h + (*str);
    str ++;
  }
  return h;
}

int CStringUtil::getPrimeHash(const char *str)
{
  int h = 0;
  while(*str) {
    h = 31*h + (*str);
    str ++;
  }
  return ((h & 0x07FFFFFFF) % 269597);
}

void CStringUtil::split(char *str, const char *delim, std::vector<char*> &list) 
{
  if (str == NULL) 
  {
    return;
  }
  if (delim == NULL) 
  {
    list.push_back(str);
    return;
  }
  
  char *s;
  const char *spanp;

  s = str;
  while(*s) 
  {
    spanp = delim;
    while(*spanp) 
    {
      if (*s == *spanp) 
      {
        list.push_back(str);
        *s = '\0';
        str = s+1;
        break;
      }
      spanp ++;
    }
    s ++;
  }
  if (*str) 
  {
    list.push_back(str);
  }
}

char *CStringUtil::urlDecode(const char *src, char *dest)
{
  if (src == NULL || dest == NULL)
  {
    return NULL;
  }
  
  const char *psrc = src;
  char *pdest = dest;
  
  while(*psrc) {
    if (*psrc == '+') {
        *pdest = ' ';
    } else if (*psrc == '%' && isxdigit(*(psrc+1)) && isxdigit(*(psrc+2))) {
        int c = 0;
        for(int i=1; i<=2; i++) {
            c <<= 4;
            if (psrc[i] >= '0' && psrc[i] <= '9') {
                c |= (psrc[i] - '0');
            } else if (psrc[i] >= 'a' && psrc[i] <= 'f') {
                c |= (psrc[i] - 'a') + 10;
            } else if (psrc[i] >= 'A' && psrc[i] <= 'F') {
                c |= (psrc[i] - 'A') + 10;
            }
        }
        *pdest = (char) (c & 0xff);
        psrc += 2;
    } else {
        *pdest = *psrc;
    }
    psrc ++;
    pdest ++;
  }
  *pdest = '\0';
  return dest;
}

unsigned int CStringUtil::murMurHash(const void *key, int len)
{
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  const int seed = 97;
  unsigned int h = seed ^ len;
  // Mix 4 bytes at a time into the hash
  const unsigned char *data = (const unsigned char *)key;
  while(len >= 4)
  {
    unsigned int k = *(unsigned int *)data;
    k *= m; 
    k ^= k >> r; 
    k *= m; 
    h *= m; 
    h ^= k;
    data += 4;
    len -= 4;
  }
  // Handle the last few bytes of the input array
  switch(len)
  {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
      h *= m;
  };
  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

std::string CStringUtil::formatByteSize(double bytes)
{
  static const char _sizeunits[] = "KMGTP";
  char s[16];
  int level = 0;
  while (bytes >= 1024.0) {
    bytes /= 1024.0;
    level ++;
    if (level >= 5) break;
  }

  if (level > 0) {
    snprintf(s, 16, "%.1f%c", bytes, _sizeunits[level-1]);
  } else {
    snprintf(s, 16, "%d", (int)bytes);
  }
  return s;
}

} //namespace base
} //namespace neptune
