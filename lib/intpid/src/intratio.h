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
    assert(r.numerator == 0 || std::abs(l) <= r.max_multiplier);
#endif
    return static_cast<int32_t>(l) * r.numerator >> r.log2_denominator;
  }

  template <typename N>
  friend N operator*(IntRatio r, N l) {
    return l * r;
  }

  inline friend std::ostream& operator<<(std::ostream& o, IntRatio r) {
    o << std::format("{} / (2^{}) ~= {}", r.numerator, r.log2_denominator,
                     1.0 * r.numerator / (1 << r.log2_denominator));
    return o;
  }

 public:
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

    const int32_t max_abs_numerator =
        std::numeric_limits<int32_t>::max() / max_multiplier;

    int32_t best_d = -1;
    int32_t best_n = 0;
    float best_approx = 0;
    float best_error = 1;
    for (int dp = 0; dp < 32; ++dp) {
      const int32_t denominator = 1 << dp;
      const float exact_numerator = value * denominator;
      if (std::abs(exact_numerator) > max_abs_numerator) {
        break;
      }
      const int32_t numerator = std::round(exact_numerator);
      const float approx = 1.0 * numerator / denominator;
      const float error = (approx - value) / std::abs(value);

      bool is_better_error;
      const float old_dist_from_zero = std::abs(best_error);
      const bool old_acceptable = old_dist_from_zero <= max_error;
      const float dist_from_zero = std::abs(error);
      const bool new_acceptable = dist_from_zero <= max_error;
      if (new_acceptable && error >= 0) {
        // Acceptable positive errors replace any negative error or any error
        // that is further from zero.
        is_better_error = best_error < 0 || dist_from_zero < old_dist_from_zero;
      } else if (new_acceptable && error < 0) {
        // Negative errors replace unacceptable errors or other negative errors
        // further from zero.
        is_better_error =
            !old_acceptable ||
            (best_error < 0 && dist_from_zero < old_dist_from_zero);
      } else {
        // Unacceptable errors only replace errors further from zero.
        is_better_error = dist_from_zero < old_dist_from_zero;
      }

      if (is_better_error) {
        best_d = dp;
        best_n = numerator;
        best_error = error;
        best_approx = approx;
      }
      if (best_error > 0 && std::abs(best_error) < stop_error) {
        break;
      }
    }

    if (best_error == 1) {
      return std::unexpected(
          "Cannot find any suitable ratio since max_multiplier is too "
          "large.\n");
    }
    if (std::abs(best_error) > max_error) {
      return std::unexpected(std::format(
          "Cannot find ratio with low enough relative error. Best "
          "approximation: "
          "{} => ({} / (2^{})) ~= {} err={} max_err={}\n",
          value, best_n, best_d, best_approx, best_error, max_error));
    }

    return IntRatio{
        .numerator = best_n,
        .log2_denominator = best_d,
#ifndef NDEBUG
        .max_multiplier = max_multiplier,
#endif
    };
  }
};

}  // namespace intpid

#endif  // INTPID_INTRATIO_H