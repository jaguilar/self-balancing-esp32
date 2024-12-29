#ifndef INTPID_INTRATIO_H
#define INTPID_INTRATIO_H

#include <cmath>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>

namespace intpid {

struct IntRatio {
  int32_t numerator;
  int32_t log2_denominator;

  template <typename N>
  friend N operator*(N l, IntRatio r) {
    return static_cast<int32_t>(l) * r.numerator >> r.log2_denominator;
  }

  template <typename N>
  friend N operator*(IntRatio r, N l) {
    return l * r;
  }

  static std::expected<IntRatio, std::string> FromFloat(
      float value, int32_t max_input, float max_error = 0.01f) {
    if (value == 0) {
      return IntRatio{.numerator = 0, .log2_denominator = 0};
    }
    if (value < 0) {
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
    const int32_t log2_denominator = i;
    const int32_t numerator = value * (1 << log2_denominator);

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
    return IntRatio{.numerator = numerator,
                    .log2_denominator = log2_denominator};
  }
};

}  // namespace intpid

#endif  // INTPID_INTRATIO_H