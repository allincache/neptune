#ifndef N_DFS_DS_VISITSTAT_H_
#define N_DFS_DS_VISITSTAT_H_

#include <set>
#include <vector>
#include <string>
#include "base/common/Internal.h"
#include "base/time/TimeUtil.h"

namespace neptune {
namespace dfs {
namespace dataserver {

uint32_t getip(const uint64_t ipport);

class VisitStat
{
  public:
    VisitStat() :
      last_vs_time_(time(NULL)), last_dump_vs_time_(time(NULL))
    {
    }
    ~VisitStat()
    {
    }

  public:
    void check_visit_stat();
    int stat_visit_count(const int32_t size);

  private:
    void dump_visit_stat();
    int incr_visit_count(const int32_t key, const int64_t count);
    int32_t filesize_category(const int32_t size);
    std::string filesize_category_desc(const int32_t category);

  private:
    static const int32_t STAT_BUF_LEN = 32;
  private:
    int last_vs_time_;
    int last_dump_vs_time_;

    std::map<int32_t, int64_t> cache_hit_map_; // cache hit stat.
    std::map<int32_t, int64_t> visit_stat_map_; // visit count stat.
};

class AccessStat
{
  public:
    typedef __gnu_cxx ::hash_map<uint32_t, Throughput> ThroughputStat;
    enum ThroughputType
    {
      READ_COUNT = 1,
      READ_BYTES,
      WRITE_COUNT,
      WRITE_BYTES
    };

  public:
    int32_t incr(const uint64_t ipport, const ThroughputType type, const int64_t count);
    Throughput get_value(const uint32_t ip) const;

    const ThroughputStat & get_stat() const
    {
      return stats_;
    }
  private:
    Throughput& get_ref(uint32_t key);
  private:
    ThroughputStat stats_;
};

class AccessControl
{
  public:
    enum AclFlagType
    {
      ACL_FLAG = 1,
      ACL_IPMASK,
      ACL_IPLIST,
      ACL_CLEAR,
      ACL_RELOAD
    };

    enum AclOperType
    {
      READ = 0x1,
      WRITE = 0x2,
      UNLINK = 0x4
    };

    struct IpMask
    {
        IpMask() :
          ip(0), mask(0)
        {
        }

        IpMask(uint32_t i, uint32_t m) :
          ip(i), mask(m)
        {
        }

        uint32_t ip;
        uint32_t mask;
    };

  public:
    AccessControl();
    ~AccessControl();

  public:
    int32_t load();
    int32_t reload();
    int32_t load(const char* aclipmask, const char* aclfile);
    bool deny(uint64_t ipport, int32_t op);
    int32_t set_flag(int32_t f)
    {
      flag_ = f;
      return flag_;
    }
    int32_t insert_ipmask(uint32_t ip, uint32_t mask);
    int32_t insert_iplist(uint32_t ip);
    int32_t clear();

  private:
    int32_t flag_;
    std::vector<IpMask> acl_;
    std::set<uint32_t> ipset_;
  private:
    static int read_file_ip_list(const char* filename, std::set<uint32_t>& hosts);
};

#define TIMER_START()\
TimeStat time_stat;\
time_stat.start()

#define TIMER_END() time_stat.end()
#define TIMER_DURATION() time_stat.duration()

class TimeStat
{
  public:
    TimeStat() : start_(0), end_(0)
    {
    }
    ~TimeStat()
    {
    }

    inline void start()
    {
      start_ = CTimeUtil::getTime();
    }

    inline void end()
    {
      end_ = CTimeUtil::getTime();
    }

    inline int64_t duration()
    {
      return end_ - start_;
    }

  private:
    int64_t start_;
    int64_t end_;
};

}
}
}

#endif //N_DFS_DS_VISITSTAT_H_
