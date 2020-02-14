#ifndef NEP_BASE_NETWORK_N_H
#define NEP_BASE_NETWORK_N_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#ifndef SHUT_RD
  #define SHUT_RD 0
#endif

#ifndef SHUT_WR
  #define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
  #define SHUT_RDWR 2
#endif

namespace neptune {
namespace base {

bool interrupted();

int setBlock( SOCKET fd , bool block );

int createPipe(SOCKET fds[2]);

int closeSocketNoThrow( SOCKET fd );

} //namespace base
} //namespace neptune

#endif //NEP_BASE_NETWORK_N_H
