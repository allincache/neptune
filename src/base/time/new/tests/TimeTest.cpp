#include <stdint.h>
#include <time.h>
#include <limits>
#include <string>
#include <gtest/gtest.h>
#include "base/time/new/Time.h"


namespace base {

TEST(TimeTest, time) {
  base::Time result;
  EXPECT_TRUE(result.is_null());
  EXPECT_TRUE(result.is_null());
  
}


}  // namespace base
