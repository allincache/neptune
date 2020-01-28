#ifndef N_DFS_DS_CPUMETRICS_H
#define N_DFS_DS_CPUMETRICS_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

namespace neptune {
namespace dfs {
namespace dataserver {

class CpuMetrics
{
 public:
  static const uint32_t MAX_CPU_COUNT = 20;
  typedef uint64_t TIC_t;
  typedef int64_t SIC_t;
  struct CpuInfo
  {
      TIC_t u, n, s, i, w, x, y, z; // as represented in /proc/stat
      TIC_t u_sav, s_sav, n_sav, i_sav, w_sav, x_sav, y_sav, z_sav;
      int32_t id;
  };

  struct UsageInfo
  {
      float u, n, s, i, w, x, y, z; // percent
  };

  CpuMetrics();
  ~CpuMetrics();

  inline bool is_valid() const
  {
    return valided_;
  }

  int32_t summary();
  inline uint32_t get_cpu_count() const
  {
    return cpu_count_;
  }
  inline const UsageInfo& get_usage(const uint32_t index) const
  {
    return usage_data_[index];
  }

 private:
  bool valided_;
  FILE* fp_;
  uint32_t cpu_count_;
  CpuInfo cpu_data_[MAX_CPU_COUNT];
  UsageInfo usage_data_[MAX_CPU_COUNT];

  int save(const uint32_t index);
  int calc(const uint32_t index, UsageInfo& ret);
  int saveall();
  int calcall();
  int refresh();
  int dump(const uint32_t index);
};

}
}
}

#endif //N_DFS_DS_CPUMETRICS_H
