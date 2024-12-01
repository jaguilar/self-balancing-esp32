#include "intpid.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <expected>
#include <limits>

#include "intratio.h"

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

#if 0
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
  const float ki = config.gain / config.integral_time;
  const float kd = config.gain * config.derivative_time;

  const float output_range = config.output_max - config.output_min;

  // The integrator sum is clamped such that the integrator will never exceed
  // the output range in a pi controller. The d term is expected to typically
  // be relatively small and is thus is not considered in this calculation.
  const float integrator_clamp = 2 * output_range * kp / ki;

  // Compute the maximum absolute error we can observe.
  const uint32_t max_abs_err =
      std::max<uint32_t>(config.setpoint_max - config.measurement_min,
                         config.measurement_max - config.setpoint_min);
  const int32_t max_output_magnitude = std::max<int32_t>(
      std::abs(config.output_min), std::abs(config.output_max));

  const int32_t antiwindup_limit = max_output_magnitude * 2 / config.gain;

  // Compute the IntRatios for kP, kI, and kD.
  IntRatio kpr, kir, kdr;
  if (auto r = IntRatio::FromFloat(kp, max_abs_err); r.has_value()) {
    kpr = *r;
  } else {
    return std::unexpected<std::string>(r.error() +
                                        "; while computing kP ratio");
  }
  if (auto r = IntRatio::FromFloat(ki, integrator_clamp); r.has_value()) {
    kir = *r;
  } else {
    return std::unexpected<std::string>(r.error() +
                                        "; while computing kI ratio");
  }
  if (auto r = IntRatio::FromFloat(kd, 2 * max_abs_err); r.has_value()) {
    kdr = *r;
  } else {
    return std::unexpected<std::string>(r.error() +
                                        "; while computing kD ratio");
  }
  float a = 0.3;
  IntRatio derr_a, derr_1_minus_a;
  if (auto r = IntRatio::FromFloat(a, max_abs_err); r.has_value()) {
    derr_a = *r;
  } else {
    return std::unexpected<std::string>(r.error() + "; while computing derr_a");
  }
  if (auto r = IntRatio::FromFloat(1 - a, max_abs_err); r.has_value()) {
    derr_1_minus_a = *r;
  } else {
    return std::unexpected<std::string>(r.error() + "; while computing derr_a");
  }

  return Pid(kpr, kir, kdr, derr_a, derr_1_minus_a, integrator_clamp);
}

int32_t Pid::Update(int32_t setpoint, int32_t measurement, int32_t dt) {
  const int32_t err = measurement - setpoint;
  const int32_t raw_derr = err - prev_err_;
  prev_err_ = err;
  derr_ = raw_derr * derr_a_ + derr_ * derr_1_minus_a_;
  integrator_ =
      std::clamp(integrator_ + err, -integrator_clamp_, integrator_clamp_);

  return 0;
}

#endif

}  // namespace intpid