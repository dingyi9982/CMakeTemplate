#include <gtest/gtest.h>

#define private public
#define protected public

#include "demo.h"

TEST(Demo, GetData) {
  EXPECT_EQ(GetData(), 1);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
