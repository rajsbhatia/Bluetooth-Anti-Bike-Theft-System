#include <Wire.h>
#include <SoftWire.h>

const int mpuAddr = 0x68;
int16_t axisX, axisY, axisZ;
int buzzPin = 16;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

double currX;
double currY;
double currZ;

void setup() {
  Wire.begin();
  Wire.beginTransmission(mpuAddr);
  Wire.write(0x6B);
  unsigned char temp = 0x0;
  Wire.write(temp);
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(buzzPin, OUTPUT);
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

  //Serial.println("***********");
  //Serial.println(x);
  //Serial.println(y);
  //Serial.println(z);
  //delay(1000);

  bool check = true;
  int tempDiff = 0;
  for (int i = 0; i < 3; i++){
    if (i == 0){
      tempDiff = currX - x;
    }
    else if (i == 1){
      tempDiff = currY - y;
    }
    else if (i == 2){
      tempDiff = currZ - z;
    }
    
    if ((tempDiff >= 100) || (tempDiff <= -100)){
      digitalWrite(buzzPin, HIGH);
      check = false;
      Serial.println("******start*****");
      Serial.println(x);
      Serial.println(y);
      Serial.println(z);
      Serial.println("******middle*****");
      Serial.println(currX);
      Serial.println(currY);
      Serial.println(currZ);
      Serial.println("******end*****");
    }
  }

  if (check){
    digitalWrite(buzzPin, LOW);
  }
}
