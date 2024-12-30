#include "intratio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace intpid {

namespace {

TEST(IntRatio, BasicTest) {
  IntRatio r = *IntRatio::FromFloat(0.275, 1000, 0.01, 0.000001);
  // Note: this won't always work out -- in 50% of cases, the approximation will
  // be less than the target of the approximation.
  EXPECT_EQ(1000 * r, 275) << r;
}

TEST(IntRatio, SemiFuzzTest) {
  int success = 0;
  constexpr int attempts = 100'000;

  for (int i = 0; i < attempts; ++i) {
    const int32_t numerator = std::rand() % 1000;
    const int32_t denominator = std::rand() % 1000;
    const int32_t value = (std::rand() % 2000) - 1000;
    if (denominator == 0) continue;

    const float true_ratio = 1.0 * numerator / denominator;

    constexpr float max_error = 0.001;
    auto approx_ratio = IntRatio::FromFloat(true_ratio, 1000, max_error);
    if (!approx_ratio) continue;

    const float approx_ratiof =
        1.0 * approx_ratio->numerator / (1 << approx_ratio->log2_denominator);
    const int32_t true_result = std::round(value * true_ratio);
    const int32_t approx_result = value * *approx_ratio;
    const int32_t diff = std::abs(true_result - approx_result);

    // With floats, the difference between the value multplied by true ratio or
    // approx ratio could be value * the difference between the approximations.
    // However, after integer conversion, an arbitrary pair of floats may be on
    // the opposite sides of a given integral number.
    const int32_t max_expected_diff =
        std::ceil(std::abs(value * (approx_ratiof - true_ratio))) + 1;

    EXPECT_LE(diff, max_expected_diff)
        << "value: " << value << " true: " << true_result
        << " approx: " << approx_result << " true ratio: " << true_ratio
        << " approx ratio: " << *approx_ratio;
    ++success;
  }

  EXPECT_GT(success, attempts * 0.9);
}

TEST(IntRatio, TooLargeMultiplier) {
  auto r = IntRatio::FromFloat(2, std::numeric_limits<int32_t>::max() - 1);
  EXPECT_FALSE(r);
  EXPECT_THAT(r.error(), testing::HasSubstr("too large"));
}

TEST(IntRatio, NoRatioWithinMaxError) {
  auto r = IntRatio::FromFloat(12345.12312414, 100000, 0.000000000000000000001);
  ASSERT_FALSE(r) << "Expected error, but got successful ratio: " << *r;
  EXPECT_THAT(r.error(), testing::HasSubstr("with low enough relative error"));
}

}  // namespace
}  // namespace intpid
