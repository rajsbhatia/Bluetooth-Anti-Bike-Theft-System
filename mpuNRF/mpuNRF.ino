#include <Wire.h>
#include <SPI.h>
#include <BLEPeripheral.h>

// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

// LED pin
#define LED_PIN   22

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

bool armed = false;

// create peripheral instance, see pinouts above
BLEPeripheral            blePeripheral        = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
BLEService               ledService           = BLEService("19b10000e8f2537e4f6cd104768a1214");

// create switch characteristic
BLECharCharacteristic    switchCharacteristic = BLECharCharacteristic("19b10001e8f2537e4f6cd104768a1214", BLERead | BLEWrite);

bool isArmed(){
  if (armed){
    readMPU6050();
  }
}

void readMPU6050(){
  Wire.beginTransmission(mpuAddr);
  Wire.write(0x43);
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
    
    if ((tempDiff >= 350) || (tempDiff <= -350)){
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
  currX = x;
  currY = y;
  currZ = z;
}

void setup() {
  Wire.begin();
  
  Wire.beginTransmission(mpuAddr);
  Wire.write(0x6B);
  unsigned char temp = 0x0;
  Wire.write(temp);
  Wire.endTransmission(true);
  
  Serial.begin(9600);
  
  pinMode(buzzPin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

    // set advertised local name and service UUID
  blePeripheral.setLocalName("LED");
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());

  // add service and characteristic
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(switchCharacteristic);

  // begin initialization
  blePeripheral.begin();

  Serial.println(F("BLE LED Peripheral"));
}

void loop() {
  BLECentral central = blePeripheral.central();
  
  if (central) {
    // central connected to peripheral
    Serial.print(F("Connected to central: "));
    Serial.println(central.address());

    while (central.connected()) {
      // central still connected to peripheral
      if (switchCharacteristic.written()) {
        // central wrote new value to characteristic, update LED
        if (switchCharacteristic.value()) {
          Serial.println(F("LED on"));
          digitalWrite(LED_PIN, HIGH);
          armed = true;
        } else {
          Serial.println(F("LED off"));
          digitalWrite(LED_PIN, LOW);
          armed = false;
        }
      }
      isArmed();
    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }

  isArmed();

}
