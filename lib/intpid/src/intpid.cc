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
  Pid pid(config);
  return pid;
}

SQ15x16 Pid::Update(SQ15x16 measurement, SQ15x16 dt) {
  if (dt <= 0) {
    prev_measurement_ = measurement;
    return 0;
  }
  const SQ15x16 err = setpoint_ - measurement;
  const SQ15x16 prev_err = setpoint_ - prev_measurement_;
  derr_ = err - prev_err;
  prev_measurement_ = measurement;

  const SQ15x16 p = kp_ * err;
  const SQ15x16 d = kd_ * derr_;
  const SQ15x16 pd = p + d;
  const SQ15x16 err_time = err * dt;
  const SQ15x16 di = err_time * ki_;
  i_sum_ += di;

  SQ15x16 sum = pd + i_sum_;
  if (sum > output_max_) {
    if (sum > integrator_upper_cutoff_) {
      // Anti-windup: prevent the integrator from going far higher than
      // needed to saturate the output. This just undoes the increment we
      // did earlier.
      i_sum_ -= di;
    }
    sum = output_max_;
  } else if (sum < output_min_) {
    if (sum < integrator_lower_cutoff_) {
      i_sum_ -= di;
    }
    sum = output_min_;
  }

#if INTPID_SUPPRESS_LOGGING == 0
  measurement_ = measurement;
  p_ = p;
  i_ = i_sum_;
  d_ = d;
  sum_ = sum;
#endif
  return sum;
}

}  // namespace intpid