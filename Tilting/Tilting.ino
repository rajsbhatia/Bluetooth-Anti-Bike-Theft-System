#include <Arduino_LSM9DS1.h>

float x, y, z, delta = 400.0;

void setup() {
  Serial.begin(9600);
  IMU.begin();
}

void loop() {
  if (IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(x, y, z);

    if(y < -delta)
      Serial.println("Flicked down");
    else if(y > delta)
      Serial.println("Flicked up");
    else if(x < -delta)
      Serial.println("Rotated left");
    else if(x > delta)
      Serial.println("Rotated right");
    else if(z < -delta)
      Serial.println("Flicked right");
    else if(z > delta)
      Serial.println("Flicked left");
  }
}
