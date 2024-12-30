#include <cassert>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <string>

#include "intratio.h"

namespace intpid {

struct Config {
  // Also known as kP. If integral_time and derivative_time are zero, update
  // will return gain * (setpoint - measurement).
  float gain;

  // The integral time is the duration after which the integral term would be
  // as large as the proportional term for a fixed error value.
  //
  // If the value is zero or less, the integral term is not used.
  float integral_time;

  // Given a fixed rate of change in the error (derr), starting from zero, how
  // long take for gain * err to equal kD * derr?
  //
  // If the value is zero or less, the derivate term is not used.
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

  // Adjusts the setpoint. This must be called once before the first call
  // to Update.
  void set_setpoint(int32_t setpoint) {
    const int32_t setpoint_diff = setpoint - setpoint_;
    setpoint_ = setpoint;
    // Prevent spikes in the d term by adjusting the previous error
    // to reflect the new setpoint.
    prev_err_ += setpoint_diff;
  }

  int32_t Update(int32_t measurement, int32_t dt);

 private:
  Pid(IntRatio kp, IntRatio ki, IntRatio kd, int32_t integrator_clamp,
      int32_t output_min, int32_t output_max)
      : kp_(kp),
        ki_(ki),
        kd_(kd),
        integrator_clamp_(integrator_clamp),
        output_min_(output_min),
        output_max_(output_max) {}

  struct ErrorInfo {
    int32_t err;
    int32_t derr;
  };

  const IntRatio kp_, ki_, kd_;
  const int32_t integrator_clamp_;
  const int32_t output_min_, output_max_;

  int32_t setpoint_ = 0;

  int32_t integrator_ = 0;
  int32_t prev_err_ = 0;

  // derr_ is a smoothed view of the derror term.
  int32_t derr_ = 0;

#if INTPID_SUPPRESS_LOGGING == 0
 public:
  // If logging is enabled, we record the values we saw for the
  // setpoint, measurement, and the calculated P I and D terms.
  // We also expose the state of the integrator and derr.
  // These values are for testing only.
  int32_t setpoint() const { return setpoint_; }
  int32_t measurement() const { return measurement_; }
  int32_t p() const { return p_; }
  int32_t i() const { return i_; }
  int32_t d() const { return d_; }
  int32_t derr() const { return derr_; }
  int32_t integrator() const { return integrator_; }
  int32_t sum() const { return sum_; }

 private:
  int32_t measurement_ = 0;
  int32_t p_ = 0;
  int32_t i_ = 0;
  int32_t d_ = 0;
  int32_t sum_ = 0;
#endif
};

}  // namespace intpid