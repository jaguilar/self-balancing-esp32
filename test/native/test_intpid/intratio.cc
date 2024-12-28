#include "intratio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace intpid {

namespace {

TEST(IntRatio, BasicTest) {
  IntRatio r = *IntRatio::FromFloat(0.25, 1000);
  EXPECT_EQ(r.numerator, 2097152);
  EXPECT_EQ(r.log2_denominator, 23);
  EXPECT_EQ(1000 * r, 250);
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

#if defined(ARDUINO)
#include <Arduino.h>

void setup() {
  // should be the same value as for the `test_speed` option in "platformio.ini"
  // default value is test_speed=115200
  Serial.begin(115200);

  ::testing::InitGoogleTest();
  // if you plan to use GMock, replace the line above with
  // ::testing::InitGoogleMock();
}

void loop() {
  // Run tests
  if (RUN_ALL_TESTS())
    ;

  // sleep for 1 sec
  delay(1000);
}

#else
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // if you plan to use GMock, replace the line above with
  // ::testing::InitGoogleMock(&argc, argv);

  if (RUN_ALL_TESTS())
    ;

  // Always return zero-code and allow PlatformIO to parse results
  return 0;
}
#endif
