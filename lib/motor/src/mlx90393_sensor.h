#ifndef MOTOR_MLX90393_SENSOR_H
#define MOTOR_MLX90393_SENSOR_H

#include <Arduino.h>

#include "Adafruit_MLX90393.h"
#include "sensor.h"

namespace motor {

class MLX90393Sensor : public Sensor {
 public:
  MLX90393Sensor(Adafruit_MLX90393* sensor) : sensor_(sensor) {}
  ~MLX90393Sensor() override {}

  void Update();

  // Returns the current angle of the in decidegrees. Note that this is the
  // total accumulated angle since startup. The absolute angle is not provided.
  int angle() override { return angle_; }

  // Returns the rate of rotation in degrees/sec during the last sensing
  // period. Positive is clockwise.
  int rate() override { return speed_; }

  void SetAngle(int angle) override { angle_ = angle; };

 private:
  Adafruit_MLX90393* const sensor_;

  // Last angle reading in decidegrees.
  int64_t t_ = micros();
  int rawangle_ = 0;  // The last sensed angle.
  int angle_ = 0;     // The total angle accumulated since startup.
  int speed_ = 0;
};

}  // namespace motor

#endif  // MOTOR_MLX90393_SENSOR_H