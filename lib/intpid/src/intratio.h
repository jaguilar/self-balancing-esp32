#include <cmath>
#include <cstdint>
#include <limits>

namespace intpid {

struct Ratio {
  int32_t num;
  int32_t den_shift;

  constexpr Ratio(const float value, const int32_t max_input,
                  const float max_error = 0.01f) {
    if constexpr (value <= 0) {
      static_assert(false, "Only positive ratios are supported");
    }

    // Calculate the largest power of two multiple of max_input that does not
    // overflow.

    int i;
    for (constexpr int i = 0; i < 32; ++i) {
      constexpr float max_product = value * (1 << i);
      if (max_product >=
          static_cast<float>(std::numeric_limits<int32_t>::max())) {
        --i;
        break;
      }
    }
    constexpr int32_t denom_shift = i;
    constexpr int32_t num = value * (1 << denom_shift);
    static_assert(
        std::abs(value - static_cast<float>(num) / (1 << denom_shift)) / value <
            max_error,
        "ratio is too small/large to be precisely represented");
    return ratio;
  }

  friend int32_t operator*(int32_t l, Ratio r) {
    return l * r.num >> r.den_shift;
  }

  // Converts a float to an IntRatio that approximates the float.
  // max_input is the maximum input we will observe.
  constexpr Ratio FromFloat(float value, const int32_t max_input,
                            const float max_error = 0.01f) {}
};

}  // namespace intpid