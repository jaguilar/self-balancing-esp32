#include <cmath>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>

namespace intpid {

struct IntRatio {
  int32_t numerator;
  int32_t log2_denominator;

  friend int32_t operator*(int32_t l, IntRatio r) {
    return l * r.numerator >> r.log2_denominator;
  }

  static std::expected<IntRatio, std::string> FromFloat(
      float value, int32_t max_input, float max_error = 0.01f) {
    if (value <= 0) {
      return std::unexpected(
          std::format("Value must be positive: {}\n", value));
    }

    int i;
    for (i = 0; i < 32; ++i) {
      float max_product = static_cast<float>(max_input) * value * (1 << i);
      if (max_product >=
          static_cast<float>(std::numeric_limits<int32_t>::max())) {
        --i;
        break;
      }
    }
    if (i < 0) {
      return std::unexpected(
          std::format("ratio * max_value too large : {}\n",
                      static_cast<float>(max_input) * value));
    }
    int32_t denom_shift = i;
    int32_t num = value * (1 << denom_shift);

    // Verify we've found a close enough ratio, or else we'll fail.
    float calculated_ratio = static_cast<float>(num) / (1 << denom_shift);
    float relative_error = std::abs(calculated_ratio - value) / value;
    if (relative_error > max_error) {
      return std::unexpected(
          std::format("Cannot find ratio with low enough relative error: "
                      "{} => ({} / (2^{})) = {} err={} max_err={}\n",
                      value, num, denom_shift, calculated_ratio, relative_error,
                      max_error));
    }
    return IntRatio{.numerator = num, .log2_denominator = denom_shift};
  }
};

}  // namespace intpid