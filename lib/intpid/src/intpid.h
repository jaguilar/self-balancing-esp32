#include <cassert>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <string>

namespace intpid {

#if 0

struct Config {
  // Also known as kP. If integral_time and derivative_time are zero, update
  // will return gain * (setpoint - measurement).
  float gain;

  // The integral time is the duration after which the integral term would be
  // as large as the proportional term for a fixed error value.
  float integral_time;

  // Given a fixed rate of change in the error (derr), starting from zero, how
  // long take for gain * err to equal kD * derr?
  float derivative_time;

  // The minimum and maximum setpoint.
  int32_t setpoint_min;
  int32_t setpoint_max;

  // The minimum and maximum measurements we will observe.
  int32_t measurement_min;
  int32_t measurement_max;

  // The minimum and maximum outputs.
  int32_t output_min;
  int32_t output_max;

  // The maximum dt that will be passed to update.
  int32_t timestep_max;
};

class Pid {
 public:
  static std::expected<Pid, std::string> Create(const Config& config);

  int32_t Update(int32_t setpoint, int32_t measurement, int32_t dt);

 private:
  Pid(IntRatio kp, IntRatio ki, IntRatio kd, IntRatio derr_a,
      IntRatio derr_1_minus_a, int32_t integrator_clamp)
      : kp_(kp),
        ki_(ki),
        kd_(kd),
        derr_a_(derr_a),
        derr_1_minus_a_(derr_1_minus_a),
        integrator_clamp_(integrator_clamp) {}

  struct ErrorInfo {
    int32_t err;
    int32_t derr;
  };

  const IntRatio kp_, ki_, kd_;
  const IntRatio derr_a_, derr_1_minus_a_;
  const int32_t integrator_clamp_;

  int32_t integrator_ = 0;
  int32_t prev_err_ = 0;

  // derr_ is a smoothed view of the derror term.
  int32_t derr_ = 0;
};

#endif

}  // namespace intpid