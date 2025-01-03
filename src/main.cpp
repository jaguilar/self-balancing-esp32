#include "Adafruit_MLX90393.h"
#include "mlx90393_sensor.h"
#include "three_wire_motor.h"

constexpr int pin_pwma = 0;
constexpr int pin_ain1 = 1;
constexpr int pin_ain2 = 8;

Adafruit_MLX90393 sensor = Adafruit_MLX90393();
motor::MLX90393Sensor motor_sensor(&sensor);
motor::ThreeWireMotor motor1(pin_pwma, pin_ain1, pin_ain2);

#define MLX90393_CS 10

double VectorToAngle(double x, double y) {
  if (x == 0.0 && y == 0.0) {
    return 0.0;  // Angle is undefined for the zero vector
  } else if (x == 0.0 && y > 0.0) {
    return 90.0;  // Angle is 90 degrees
  } else if (x == 0.0 && y < 0.0) {
    return 270.0;  // Angle is 270 degrees
  }

  double angle = atan2(y, x) * 180 / PI;
  if (angle < 0) {
    angle = 360 + angle;
  }
  return angle;
}

constexpr uint8_t i2c_addr = 0x18;

void setup(void) {
  analogWriteFrequency(pin_pwma, 128);
  analogWriteResolution(pin_pwma, 8);

  motor1.Begin();

  xTaskCreate(
      +[](void*) {
        motor::Direction direction = motor::kClockwise;
        motor::Direction other_direction = motor::kCounterClockwise;

        while (true) {
          delay(1000);
          motor1.SetDirection(direction);
          delay(1000);
          motor1.SetDuty(256 * 0.5);
          delay(1000);
          motor1.SetDuty(256 * 0.75);
          delay(1000);
          motor1.SetDuty(256 * 1);
          delay(1000);
          motor1.Stop(motor::kBrake);
          delay(500);
          motor1.Stop(motor::kCoast);
          std::swap(direction, other_direction);
        }
      },
      "motor", 1024, NULL, 1, NULL);

  Serial.begin(115200);

  /* Wait for serial on USB platforms. */
  while (!Serial) {
    delay(10);
  }

  Serial.println("Starting Adafruit MLX90393 Demo");

  Wire.setPins(4, 5);

  // hardware I2C mode, can pass in address & alt Wire
  while (!sensor.begin_I2C(i2c_addr)) {
    // if (! sensor.begin_SPI(MLX90393_CS)) {  // hardware SPI mode
    Serial.println("No sensor found ... check your wiring?");
    delay(5000);
  }
  Serial.println("Found a MLX90393 sensor");

  sensor.setGain(MLX90393_GAIN_1X);
  // You can check the gain too
  Serial.print("Gain set to: ");
  switch (sensor.getGain()) {
    case MLX90393_GAIN_1X:
      Serial.println("1 x");
      break;
    case MLX90393_GAIN_1_33X:
      Serial.println("1.33 x");
      break;
    case MLX90393_GAIN_1_67X:
      Serial.println("1.67 x");
      break;
    case MLX90393_GAIN_2X:
      Serial.println("2 x");
      break;
    case MLX90393_GAIN_2_5X:
      Serial.println("2.5 x");
      break;
    case MLX90393_GAIN_3X:
      Serial.println("3 x");
      break;
    case MLX90393_GAIN_4X:
      Serial.println("4 x");
      break;
    case MLX90393_GAIN_5X:
      Serial.println("5 x");
      break;
  }

  // Set resolution, per axis. Aim for sensitivity of ~0.3 for all axes.
  sensor.setResolution(MLX90393_X, MLX90393_RES_17);
  sensor.setResolution(MLX90393_Y, MLX90393_RES_17);
  sensor.setResolution(MLX90393_Z, MLX90393_RES_16);

  // Set oversampling
  sensor.setOversampling(MLX90393_OSR_2);

  // Set digital filtering
  sensor.setFilter(MLX90393_FILTER_3);

  sensor.setTrigInt(true);

  if (!sensor.exitMode()) {
    Serial.print("Failed to exit mode.");
  }

  if (!sensor.setBurstRate(0)) {
    Serial.println("Failed to set burst rate");
  };

  if (!sensor.startBurstMode(MLX90393_X | MLX90393_Y)) {
    Serial.println("Failed to start burst mode");
  }
}

void loop(void) {
  motor_sensor.Update();

  Serial.printf(">ANGLE:%d\n>SPEED:%d\n", motor_sensor.angle(),
                motor_sensor.rate());

  delay(10);
}