#include <gtest/gtest.h>

#include "intratio.h"

namespace intpid {

namespace {

TEST(IntRatio, BasicTest) { Ratio r(0.25, 1000); }

}  // namespace
}  // namespace intpid