#ifndef NEP_BASE_NETWORK_NETUTIL_H
#define NEP_BASE_NETWORK_NETUTIL_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#include <net/if.h>
#include <inttypes.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <string>

namespace neptune {
namespace base {

struct ipaddr_less {
  bool operator()(const uint64_t a, const uint64_t b) const {
    uint64_t a1 = ((a & 0xFF) << 24) | ((a & 0xFF00) << 8) | ((a & 0xFF0000) >> 8) | ((a & 0xFF000000) >> 24);
    a1 <<= 32; a1 |= ((a>>32) & 0xffff);
    uint64_t b1 = ((b & 0xFF) << 24) | ((b & 0xFF00) << 8) | ((b & 0xFF0000) >> 8) | ((b & 0xFF000000) >> 24);
    b1 <<= 32; b1 |= ((b>>32) & 0xffff);
    return (a1<b1);
  }
};

class CNetUtil {

 public:

  static uint32_t getLocalAddr(const char *dev_name);
  static bool isLocalAddr(uint32_t ip, bool loopSkip = true);
  static uint32_t getAddr(const char *ip);
  static std::string addrToString(uint64_t ipport);
  static uint64_t strToAddr(const char *ip, int port);
  static uint64_t ipToAddr(uint32_t ip, int port);
};

} //namespace base
} //namespace neptune

#endif //NEP_BASE_NETWORK_NETUTIL_H
