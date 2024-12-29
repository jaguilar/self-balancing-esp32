#include "intratio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace intpid {

namespace {

TEST(IntRatio, BasicTest) {
  IntRatio r = *IntRatio::FromFloat(0.27, 1000, 0.01, 0.000001);
  EXPECT_EQ(1000 * r, 270);
}

TEST(IntRatio, TooLargeMultiplier) {
  auto r = IntRatio::FromFloat(2, std::numeric_limits<int32_t>::max() - 1);
  EXPECT_FALSE(r);
  EXPECT_THAT(r.error(), testing::HasSubstr("too large"));
}

TEST(IntRatio, NoRatioWithinMaxError) {
  auto r = IntRatio::FromFloat(12345.12312414, 100000, 0.000000000001);
  EXPECT_FALSE(r);
  EXPECT_THAT(r.error(), testing::HasSubstr("with low enough relative error"));
}

}  // namespace
}  // namespace intpid
