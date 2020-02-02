#include "base/include/Base.h"
#include "base/common/CDefine.h"
#include <gtest/gtest.h>

TEST(CommonTest, CDefineTest_1) {
  EXPECT_EQ(SUCCESS, 0);
}

TEST(CommonTest, CDefineTest_2) {
  EXPECT_EQ(FORCE_STAT, 1);
}

