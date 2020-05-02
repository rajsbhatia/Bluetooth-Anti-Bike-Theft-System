#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service "19b10000e8f2537e4f6cd104768a1214"

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

BLEByteCharacteristic batteryCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead); // | BLENotify);

const int ledPin = LED_BUILTIN; // pin to use for the LED
const int buzzPin = 8;
const int MAX = 2147483647;
const double batteryConversion = 3600000; // conversion from ms to hours

// gryoscope values
float x, y, z;
float prevX, prevY, prevZ;
float delta = 400.0;
int Peaks[5] = {MAX, MAX, MAX, MAX, MAX};

bool armed = false;
bool password = false;

int counter = 0;
int endP = 0;

void setPins() {
  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);
}

void setBLE() {
  // begin initialization
  BLE.begin();

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(batteryCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);
  batteryCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void setGyro() {
  IMU.begin();
}

void setPassword() {
  int startTime = millis();
  int prev = -1;
  while (millis() - startTime < 10000) {
    if (IMU.gyroscopeAvailable())
    {
      IMU.readGyroscope(x, y, z);
  
      if((y < -delta) && (prev != 0)) {
        Serial.println("Flicked down");
        Peaks[endP] = 0;
        prev = 0;
        endP++;
      }
      else if((y > delta) && (prev != 1)) {
        Serial.println("Flicked up");
        Peaks[endP] = 1;
        prev = 1;
        endP++;
      }
      else if((x < -delta) && (prev != 2)) {
        Serial.println("Rotated left");
        Peaks[endP] = 2;
        prev = 2;
        endP++;
      }
      else if((x > delta) && (prev != 3)) {
        Serial.println("Rotated right");
        Peaks[endP] = 3;
        prev = 3;
        endP++;
      }
      else if((z < -delta) && (prev != 4)) {
        Serial.println("Flicked right");
        Peaks[endP] = 4;
        prev = 4;
        endP++;
      }
      else if((z > delta) && (prev != 5)) {
        Serial.println("Flicked left");
        Peaks[endP] = 5;
        prev = 5;
        endP++;
      }
      
      if (endP == 5) {
        break;
      }
    }
  }

  for (int i = 0; i < 5; i++) {
    Serial.println(Peaks[i]);
  }
  
}

void checkPassword() {
  int startTime = millis();
  int currMov = -1;
  float tempX, tempY, tempZ;
  
  while ((millis() - startTime) < 10000) {
    IMU.readGyroscope(tempX, tempY, tempZ);
    
    if(tempY < -delta) {
      Serial.println("Flicked down");
      currMov = 0;
    }
    else if(tempY > delta) {
      Serial.println("Flicked up");
      currMov = 1;
    }
    else if(tempX < -delta) {
      Serial.println("Rotated left");
      currMov = 2;
    }
    else if(tempX > delta) {
      Serial.println("Rotated right");
      currMov = 3;
    }
    else if(tempZ < -delta) {
      Serial.println("Flicked right");
      currMov = 4;
    }
    else if(tempZ > delta) {
      Serial.println("Flicked left");
      currMov = 5;
    }
    else if (currMov == -1) {
      break;
    }
    
    if (currMov == Peaks[counter]) {
      counter++;
    }
    
    Serial.println(counter);
    
    if (counter == endP) {
      digitalWrite(buzzPin, LOW);
      digitalWrite(ledPin, LOW);
      armed = false;
      password = false;
      break;
    }
  }
  
  counter = 0;
}

void resetPassword() {
  for(int i = 0; i < 5; i++) {
    Peaks[i] = MAX;
  }
  endP = 0;
}

int BatteryLife() {
  double currentTime = millis();
  double totalHours = currentTime / batteryConversion;
  double life = 70.0;
  double percentage = (1 - (totalHours / life)) * (100);
  double p = (int) percentage;
  double temp = percentage - p;

  if (temp >= 0.50) {
    return ((int)percentage + 1);
  }
  else {
    return (int)percentage;
  }
}

void readGyro() {
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
  
    bool check = true;
    int tempDiff = 0;
    for (int i = 0; i < 3; i++) {
      if (i == 0){
        tempDiff = prevX - x;
      }
      else if (i == 1){
        tempDiff = prevY - y;
      }
      else if (i == 2){
        tempDiff = prevZ - z;
      }
      
      if ((tempDiff >= 15) || (tempDiff <= -15)) {
        digitalWrite(buzzPin, HIGH);
        delay(30);
        check = false;
      }
    }
    
    if (check) {
      digitalWrite(buzzPin, LOW);
    }
    
    prevX = x;
    prevY = y;
    prevZ = z;

    if (password) {
      checkPassword();
    }
    
  }
}

bool isArmed() {
  if(armed) {
    readGyro();
  }
}

void setup() {
  Serial.begin(9600);
  
  setPins();
  setBLE();
  setGyro();
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // send the battery life to phone
      batteryCharacteristic.writeValue(BatteryLife());
      
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value() == 1) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
          armed = true;
          if (Peaks[0] != MAX) {
            password = true; 
          }
        } else if (switchCharacteristic.value() == 2) {
          Serial.println("Setting user password");
          resetPassword();
          setPassword();
          password = true;
          Serial.println("Done");
        } else if (switchCharacteristic.value() == 0){                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
          armed = false;
        }
      }
      isArmed();
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }

  isArmed();
}
