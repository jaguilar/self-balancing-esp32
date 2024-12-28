# PLAN

This document contains the plan for this project, broken down
into tasks granular enough that each one might take an hour or two.

* Software
  * [ ] Write and test magnetometer-sensor-based motor::Sensor class.
  * [ ] Write and test PID class.
  * [ ] Write and test platform-agnostic teleplot client.
  * [ ] Write and test FeedbackMotor class that integrates sensor and motor.
  * [ ] Write and test orientation sensing.
  * [ ] Write and test main control loop.
  * [ ] Write and test PS4 control.
* Hardware
  * [ ] Generate BOM for parts we won't manufacture and order them.
    * [ ] Wheels
    * [ ] Battery holder
  * [ ] Design motor+sensor+wheel housing.
    * [ ] Test some simplified design 
  * [ ] Design overall chassis
  * [ ] Design and implement control board
    * QTPY-S3 + TI motor controller
  * [ ] Design bot test safety holder
* [ ] Test and finish bot.