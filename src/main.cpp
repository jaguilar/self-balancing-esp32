#include "Adafruit_MLX90393.h"

Adafruit_MLX90393 sensor = Adafruit_MLX90393();
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

void setup(void) {
  Serial.begin(115200);

  /* Wait for serial on USB platforms. */
  while (!Serial) {
    delay(10);
  }

  Serial.println("Starting Adafruit MLX90393 Demo");

  Wire.setPins(4, 5);

  // hardware I2C mode, can pass in address & alt Wire
  while (!sensor.begin_I2C(0x18)) {
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
  sensor.setOversampling(MLX90393_OSR_3);

  // Set digital filtering
  sensor.setFilter(MLX90393_FILTER_5);
}

void loop(void) {
  float x, y, z;

  // get X Y and Z data at once
  if (sensor.readData(&x, &y, &z)) {
    Serial.print("X: ");
    Serial.print(x, 4);
    Serial.println(" uT");
    Serial.print("Y: ");
    Serial.print(y, 4);
    Serial.println(" uT");
    Serial.print("Z: ");
    Serial.print(z, 4);
    Serial.println(" uT");

    Serial.printf(">X:%f\n>Y:%f\n>Z:%f\n>ANGLE:%f\n", x, y, z,
                  VectorToAngle(x, y));

  } else {
    Serial.println("Unable to read XYZ data from the sensor.");
  }

  /* Or....get a new sensor event, normalized to uTesla */
  sensors_event_t event;
  sensor.getEvent(&event);
  /* Display the results (magnetic field is measured in uTesla) */
  Serial.print("X: ");
  Serial.print(event.magnetic.x);
  Serial.print(" \tY: ");
  Serial.print(event.magnetic.y);
  Serial.print(" \tZ: ");
  Serial.print(event.magnetic.z);
  Serial.println(" uTesla ");

  delay(500);
}