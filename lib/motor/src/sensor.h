#ifndef MOTOR_SENSOR_H
#define MOTOR_SENSOR_H

namespace motor {

class Sensor {
 public:
  virtual ~Sensor() {}

  // Returns the current angle of the in decidegrees, between 0 and 3600.
  virtual int angle() = 0;

  // Returns the rate of rotation in degrees/sec during the last sensing
  // period. Positive is clockwise.
  virtual int rate() = 0;

  // Sets the current angle. Calling this without arguments zeroes the sensor.
  virtual void SetAngle(int angle = 0) = 0;
};

}  // namespace motor

#endif  // MOTOR_SENSOR_H