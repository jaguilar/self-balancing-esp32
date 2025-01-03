#include "mlx90393_sensor.h"

#include <cmath>
#include <format>

namespace motor {

int VectorToAngleDecidegrees(float x, float y) {
  if (x == 0.0 && y == 0.0) {
    return 0;  // Angle is undefined for the zero vector
  } else if (x == 0.0 && y > 0.0) {
    return 900;  // Angle is 90 degrees
  } else if (x == 0.0 && y < 0.0) {
    return 2700;  // Angle is 270 degrees
  }

  double angle = atan2(y, x) * 1800 / PI;
  if (angle < 0) {
    angle = 3600 + angle;
  }
  return angle;
}

void MLX90393Sensor::Update() {
  std::array<float, 2> data;
  if (!sensor_->readMeasurement(MLX90393_X | MLX90393_Y, data)) {
    return;
  }

  // Note we only update sample time when we can successfully take a sample.
  const uint64_t t = micros();
  const int dt = t - t_;
  t_ = t;

  const int newangle = VectorToAngleDecidegrees(data[0], data[1]);
  const int delta = newangle - rawangle_;
  rawangle_ = newangle;

  if (std::abs(delta) < 1800) {
    // If the delta was less than 180, we'll assume that it's a normal move --
    // that is, the sensor did not wrap around.
    speed_ = delta;
  } else {
    // If the move was more than 180 degrees, we assume that we've wrapped
    // around. That means the *sign* of the move is the opposite delta, and the
    // magnitude is 360 - abs(delta).
    speed_ = (delta > 0 ? -3600 + delta : 3600 + delta);
  }

  // Since we want to accumulate the total angle, we add the adjusted delta.
  angle_ += speed_;

  // To arrive at turning rate per second, we need to divide by seconds. We
  // don't want to use floating point. We could multiply by 1'000'000 then
  // divide by dt, but that might overflow int32_t if the angle change was more
  // than 400 degrees (e.g. if there is a really slow sampling rate).
  //
  // To avoid this overflow we can divide by the numerator and the denominator
  // by the same amount. This means we'd now need to rotate by 400 * 2^N degrees
  // on one updated. We lose the lower N bits of dt, but it's so high resolution
  // that we don't really care.
  speed_ *= (1'000'000 >> 4);
  speed_ /= (dt >> 4);
}

}  // namespace motor