#ifndef N_DFS_UTIL_UTILITY_H_
#define N_DFS_UTIL_UTILITY_H_

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>

namespace neptune {
namespace dfs {

#define MAX_STR_FIELD_NUM 32
#define SEC2USEC 1000000
#define USEC2NSEC 1000

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

int getAbsPath(const char *pszPath, char *pszBuf, int iBufLen);
int checkCreateDir(const char *pszPath);
int checkCreateLink(const char *pszPath, const char *pszLink, int iRecreate);
int strJoin(char *pszDst, size_t sizeDst, char **ppszField, size_t sizeField, const char *pszSep);
int getHostIP(char *pszAddr, unsigned uiAddrLen);
int getExe(char *pszExe, unsigned uiExeLen);
int getExeRoot(char *pszExeRoot, unsigned uiExePathLen);

static inline uint32_t guint32p2(uint32_t uiValue)
{
  uiValue |= (uiValue >> 1);
  uiValue |= (uiValue >> 2);
  uiValue |= (uiValue >> 4);
  uiValue |= (uiValue >> 8);
  uiValue |= (uiValue >> 16);
  return uiValue + 1;
}

static inline uint64_t htonll(uint64_t ull)
{
    if (1 != htonl(1))
    {
        uint64_t ullRet = 0;
        char *pSrc = (char *)&ull;
        char *pDst = (char *)&ullRet;
        pDst[0] = pSrc[7];
        pDst[1] = pSrc[6];
        pDst[2] = pSrc[5];
        pDst[3] = pSrc[4];
        pDst[4] = pSrc[3];
        pDst[5] = pSrc[2];
        pDst[6] = pSrc[1];
        pDst[7] = pSrc[0];
        return ullRet;
    }
    return ull;
}

static inline uint64_t ntohll(uint64_t ull)
{
    if (1 != ntohl(1))
    {
        uint64_t ullRet = 0;
        char *pSrc = (char *)&ull;
        char *pDst = (char *)&ullRet;
        pDst[0] = pSrc[7];
        pDst[1] = pSrc[6];
        pDst[2] = pSrc[5];
        pDst[3] = pSrc[4];
        pDst[4] = pSrc[3];
        pDst[5] = pSrc[2];
        pDst[6] = pSrc[1];
        pDst[7] = pSrc[0];
        return ullRet;
    }
    return ull;
}

static inline int getFutureAbsTS(struct timespec *pts, unsigned uiUSec)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_usec += uiUSec % SEC2USEC;
    if (tv.tv_usec >= SEC2USEC)
    {
        pts->tv_sec = tv.tv_sec + uiUSec / SEC2USEC + 1;
        pts->tv_nsec = (tv.tv_usec - SEC2USEC) * 1000;
    }
    else
    {
        pts->tv_sec = tv.tv_sec + uiUSec / SEC2USEC;
        pts->tv_nsec = tv.tv_usec * USEC2NSEC;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif // __cplusplus

} //namespace dfs
} //namespace neptune

#endif //N_DFS_UTIL_UTILITY_H_
