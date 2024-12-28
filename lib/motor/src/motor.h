#ifndef MOTOR_MOTOR_H
#define MOTOR_MOTOR_H

#include <cstdint>

namespace motor {

class Motor {
 public:
  virtual ~Motor() {}

  enum StopMode {
    kBrake,  // If the motor controller has a short brake mode, apply it.
    kCoast,  // Don't apply extra stopping force.
  };

  // Stops the motor.
  virtual void Stop(StopMode mode = kCoast) = 0;

  enum Direction {
    kClockwise,
    kCounterClockwise,
  };

  // Sets the motor direction.
  virtual void SetDirection(Direction direction) = 0;

  // Sets the motor duty cycle.
  virtual void SetDuty(int duty) = 0;
};

}  // namespace motor

#endif  // MOTOR_MOTOR_H