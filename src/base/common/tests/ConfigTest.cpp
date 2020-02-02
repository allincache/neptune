#include "base/include/Base.h"
#include "base/common/Config.h"
#include <gtest/gtest.h>

using namespace neptune::base;

TEST(CommonTest, ConfigTest) {
  std::string conf_file = "/opt/neptune/dfs/conf/dataserver.conf";
  int32_t iret = NEP_CONFIG.load(conf_file.c_str());
  EXPECT_EQ(iret, 0);
  const char* word_dir = NEP_CONFIG.getString("public", "work_dir");
  std::string dir = "/home/neptune/dfs";
  EXPECT_STREQ(word_dir, dir.c_str());
}