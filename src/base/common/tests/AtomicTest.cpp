#include "base/include/Base.h"
#include "base/common/Atomic.h"
#include <gtest/gtest.h>

TEST(AtomicTest, SimpleTest) {
  EXPECT_TRUE(true);
  EXPECT_EQ(1, 1);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
