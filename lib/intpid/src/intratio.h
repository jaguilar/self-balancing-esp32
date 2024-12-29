#ifndef INTPID_INTRATIO_H
#define INTPID_INTRATIO_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>

namespace intpid {

struct IntRatio {
  int32_t numerator;
  int32_t log2_denominator;
#ifndef NDEBUG
  uint32_t max_multiplier;
#endif

  template <typename N>
  friend N operator*(N l, IntRatio r) {
#ifndef NDEBUG
    assert(std::abs(l) <= r.max_multiplier);
#endif
    return static_cast<int32_t>(l) * r.numerator >> r.log2_denominator;
  }

  template <typename N>
  friend N operator*(IntRatio r, N l) {
    return l * r;
  }

  // Creates an IntRatio from a float value to approximate.
  //
  // 'max_multiplier' is the maximum value you will multiply by
  // this ratio. (Overflow may occur if you use a higher value. In
  // debug mode this is checked.)
  //
  // 'max_error' is the maximum fractional error from the input value
  // that the result ratio may have. If a ratio cannot be found with
  // error less than this, then std::unexpected will be returned.
  //
  // 'stop_error' is the error ratio below which we will stop trying for
  // a better error. This will give some additional leeway for unexpectedly
  // large multipliers in release mode.
  static std::expected<IntRatio, std::string> FromFloat(
      float value, uint32_t max_multiplier, float max_error = 0.01f,
      float stop_error = 0.001f) {
    if (value == 0) {
      return IntRatio{.numerator = 0, .log2_denominator = 0};
    }
    bool negate = value < 0;
    if (negate) value = -value;

    int i;
    for (i = 0; i < 32; ++i) {
      const float max_product =
          static_cast<float>(max_multiplier) * value * (1 << i);
      if (max_product >=
          static_cast<float>(std::numeric_limits<int32_t>::max())) {
        if (i == 0) {
          return std::unexpected(
              std::format("ratio * max_value too large : {}\n",
                          static_cast<float>(max_multiplier) * value));
        }
        --i;  // This shift level was not acceptable. Go back one level.
        break;
      }
      // See if we've gotten the relative error low enough.
      const int32_t log2_denominator = i;
      const int32_t numerator =
          std::round(value * (1 << log2_denominator) + 0.5);
      const float ratio = 1.0 * numerator / (1 << log2_denominator);
      const float error = std::abs(ratio - value) / value;
      if (error < stop_error) {
        break;
      }
    }
    const int32_t log2_denominator = i;
    const int32_t numerator = std::round(value * (1 << log2_denominator) + 0.5);

    const float calculated_ratio =
        static_cast<float>(numerator) / (1 << log2_denominator);
    const float relative_error = std::abs(calculated_ratio - value) / value;
    if (relative_error > max_error) {
      return std::unexpected(
          std::format("Cannot find ratio with low enough relative error: "
                      "{} => ({} / (2^{})) ~= {} err={} max_err={}\n",
                      value, numerator, log2_denominator, calculated_ratio,
                      relative_error, max_error));
    }
    return IntRatio{
        .numerator = negate ? -numerator : numerator,
        .log2_denominator = log2_denominator,
#ifndef NDEBUG
        .max_multiplier = static_cast<uint32_t>(max_multiplier),
#endif
    };
  }
};

}  // namespace intpid

#endif  // INTPID_INTRATIO_H