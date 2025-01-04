#ifndef INTPID_INTPID_H
#define INTPID_INTPID_H

#include <FixedPointsCommon.h>

#include <cassert>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <string>

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
  float setpoint_min;
  float setpoint_max;

  // The minimum and maximum measurements we will observe.
  float measurement_min;
  float measurement_max;

  // The minimum and maximum outputs.
  float output_min;
  float output_max;
};

class Pid {
 public:
  Pid(Pid&&) = default;
  Pid& operator=(Pid&&) = default;

  static std::expected<Pid, std::string> Create(const Config& config);

  // Adjusts the setpoint. This must be called once before the first call
  // to Update.
  void set_setpoint(SQ15x16 setpoint) {
    const SQ15x16 setpoint_diff = setpoint - setpoint_;
    setpoint_ = setpoint;
    // Prevent spikes in the d term by adjusting the previous error
    // to reflect the new setpoint.
    prev_err_ += setpoint_diff;
  }

  SQ15x16 Update(SQ15x16 measurement, SQ15x16 dt);

 private:
  Pid(SQ15x16 kp, SQ15x16 ki, SQ15x16 kd, SQ15x16 integrator_clamp,
      SQ15x16 output_min, SQ15x16 output_max)
      : kp_(kp),
        ki_(ki),
        kd_(kd),
        integrator_clamp_(integrator_clamp),
        output_min_(output_min),
        output_max_(output_max) {}

  struct ErrorInfo {
    SQ15x16 err;
    SQ15x16 derr;
  };

  const SQ15x16 kp_, ki_, kd_;
  const SQ15x16 integrator_clamp_;
  const SQ15x16 output_min_, output_max_;

  SQ15x16 setpoint_ = 0;

  SQ15x16 integrator_ = 0;
  SQ15x16 prev_err_ = 0;

  // derr_ is a smoothed view of the derror term.
  SQ15x16 derr_ = 0;

#if INTPID_SUPPRESS_LOGGING == 0
 public:
  // If logging is enabled, we record the values we saw for the
  // setpoint, measurement, and the calculated P I and D terms.
  // We also expose the state of the integrator and derr.
  // These values are for testing only.
  SQ15x16 setpoint() const { return setpoint_; }
  SQ15x16 measurement() const { return measurement_; }
  SQ15x16 p() const { return p_; }
  SQ15x16 i() const { return i_; }
  SQ15x16 d() const { return d_; }
  SQ15x16 derr() const { return derr_; }
  SQ15x16 integrator() const { return integrator_; }
  SQ15x16 sum() const { return sum_; }

 private:
  SQ15x16 measurement_ = 0;
  SQ15x16 p_ = 0;
  SQ15x16 i_ = 0;
  SQ15x16 d_ = 0;
  SQ15x16 sum_ = 0;
#endif
};

}  // namespace intpid

#endif  // INTPID_INTPID_H