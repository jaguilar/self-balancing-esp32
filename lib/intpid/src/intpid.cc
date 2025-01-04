#include "intpid.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>

#ifdef ARDUINO_ARCH_ESP32
#include <esp_log.h>
#else
#include <stdio.h>
#endif

// In debug mode, we make a quick check of each ratio to ensure that the
// deviation is less than this amount. If the deviation is more than this, we
// return an error.
#ifndef INTPID_MAX_ERR
#define INTPID_MAX_ERR 0.01f
#endif

namespace intpid {

template <typename... Args>
void PidVLog(const char* format, Args&&... args) {
#ifdef ARDUINO_ARCH_ESP32
  esp_log_write(ESP_LOG_VERBOSE, "intpid", format, std::forward<Args>(args)...);
#elif INTPID_LOG_VERBOSE == 1
  printf(args...);
#endif
}

std::expected<Pid, std::string> Pid::Create(const Config& config) {
  const float kp = config.gain;
  const float ki =
      config.integral_time <= 0 ? 0 : config.gain / config.integral_time;
  const float kd =
      config.derivative_time <= 0 ? 0 : config.gain * config.derivative_time;

  // Compute the maximum absolute error we can observe.
  const float max_abs_err =
      std::abs(std::max<float>(config.setpoint_max - config.measurement_min,
                               config.measurement_max - config.setpoint_min));

  // The integrator sum is clamped to the value required to saturate the output
  // (in either direction) assuming the proportional-only term is saturating the
  // output in the opposite direction. (That is, twice the maximum output
  // magnitude divided by the integral gain.)
  const float max_output_magnitude = std::max<float>(
      std::abs((config.output_max)), (std::abs(config.output_min)));
  float integrator_clamp = 2 * max_output_magnitude / ki;
  printf("%f\n", integrator_clamp);
  if (integrator_clamp > SQ15x16::MaxValue.getInteger()) {
    integrator_clamp = SQ15x16::MaxValue.getInteger();
    printf("%f\n", integrator_clamp);
  }

  // Serial.println(
  //     std::format("kp:{} ki:{} kd:{} max_abs_err:{} integrator_clamp:{}", kp,
  //                 ki, kd, max_abs_err, integrator_clamp)
  //         .c_str());

  Pid pid(kp, ki, kd, integrator_clamp, config.output_min, config.output_max);
  pid.set_setpoint((config.setpoint_min + config.setpoint_max) / 2);
  return pid;
}

SQ15x16 Pid::Update(SQ15x16 measurement, SQ15x16 dt) {
  const SQ15x16 err = setpoint_ - measurement;
  derr_ = err - prev_err_;
  // Serial.printf(">DERR:%d\n");
  prev_err_ = err;

  const SQ15x16 p = kp_ * err;
  const SQ15x16 d = kd_ * derr_;
  const SQ15x16 pd = p + d;
  const bool pd_saturated = pd > output_max_ || pd < output_min_;
  if (!pd_saturated || (pd > 0) != (integrator_ > 0) || integrator_ == 0) {
    // This follows two anti-windup rules:
    // - If pd already saturates the output in the same direction as the
    //   integrator, or if the integrator is zero, do not update the integrator.
    // - The integrator is also restricted in the range of the
    //   integrator_clamp_.
    printf("%s", std::format("{} += {} (limited to {})\n", float{integrator_},
                             float{err}, float{integrator_clamp_})
                     .c_str());
    integrator_ =
        std::clamp(integrator_ + err, -integrator_clamp_, integrator_clamp_);

  } else {
    printf("%s", std::format("skipping integrator {} {} {}\n", float{pd},
                             float{integrator_}, float{err})
                     .c_str());
  }

  const SQ15x16 i = ki_ * integrator_;
  const SQ15x16 sum = std::clamp(p + i + d, output_min_, output_max_);
#if INTPID_SUPPRESS_LOGGING == 0
  measurement_ = measurement;
  p_ = p;
  i_ = i;
  d_ = d;
  sum_ = sum;
#endif

  return sum;
}

}  // namespace intpid