#include <Wire.h>
#include <SoftWire.h>

const int mpuAddr = 0x68;
int16_t axisX, axisY, axisZ;

int minVal = 265;
int maxVal = 402;

uint8_t sdaPin = 0;
uint8_t sclPin = 1;

SoftWire i2c(sdaPin, sclPin);

double x;
double y;
double z;

void readMPU()
{
  
}

void setup() {
  Wire.begin();
  Wire.beginTransmission(mpuAddr);
  Wire.write(0x6B);
  unsigned char temp = 0x0;
  Wire.write(temp);
  Wire.endTransmission(true);
  Serial.begin(9600);
}

void loop() {
  Wire.beginTransmission(mpuAddr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(mpuAddr, 14, true);
  
  axisX = Wire.read()<<8|Wire.read();
  axisY = Wire.read()<<8|Wire.read();
  axisZ = Wire.read()<<8|Wire.read();

  int xAng = map(axisX, minVal, maxVal, -90, 90);
  int yAng = map(axisY, minVal, maxVal, -90, 90);
  int zAng = map(axisZ, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

  Serial.println("***********");
  Serial.println(x);
  Serial.println(y);
  Serial.println(z);
  delay(1000);
}
