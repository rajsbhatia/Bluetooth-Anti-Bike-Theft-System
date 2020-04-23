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
int xPeaks[5] = {MAX, MAX, MAX, MAX, MAX};
int yPeaks[5] = {MAX, MAX, MAX, MAX, MAX};
int zPeaks[5] = {MAX, MAX, MAX, MAX, MAX};

bool armed = false;
bool order = false;
int counter = 0;
int endP = 0;

void setPins() {
  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);
  //pinMode(buzzPin, OUTPUT);
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
  
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");
}

void setPassword() {
  int startTime = millis();
  int x2 = 0;
  int y2 = 0;
  int z2 = 0;
  int tempX = 0;
  int tempY = 0;
  int tempZ = 0;
  
  while (millis() - startTime < 11000) {
    IMU.readGyroscope(x, y, z);

    tempX = x - x2;
    tempY = y - y2;
    tempZ = z - z2;
   
    if ((tempX >= 80) || (tempX <= -80) || (tempY >= 80) || (tempY <= -80) || (tempZ >= 80) || (tempZ <= -80)){
      xPeaks[counter] = tempX;
      yPeaks[counter] = tempY;
      zPeaks[counter] = tempZ;

      /*Serial.println("Peak");
      Serial.println(counter);
      Serial.println(tempX);
      Serial.println(tempY);
      Serial.println(tempZ);*/
      
      counter++;
      endP = counter;
      if (counter > 4) {
        break;
      }
    }
    
    x2 = x;
    y2 = y;
    z2 = z;
  }
  counter = 0;
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

    /*Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);*/
  }
  
  bool check = true;
  int tempDiff = 0;
  int tempX = 0;
  int tempY = 0;
  int tempZ = 0;
  for (int i = 0; i < 3; i++){
    if (i == 0){
      tempDiff = prevX - x;
      tempX = prevX - x;
    }
    else if (i == 1){
      tempDiff = prevY - y;
      tempY = prevY - y;
    }
    else if (i == 2){
      tempDiff = prevZ - z;
      tempZ = prevZ - z;
    }
    
    if ((tempDiff >= 15) || (tempDiff <= -15)) {
      digitalWrite(buzzPin, HIGH);
      delay(30);
      check = false;
      /*Serial.println("******start*****");
      Serial.println(x);
      Serial.println(y);
      Serial.println(z);
      Serial.println("******middle*****");
      Serial.println(prevX);
      Serial.println(prevY);
      Serial.println(prevZ);
      Serial.println("******end*****");*/
    }
  }
  
  if (check) {
    digitalWrite(buzzPin, LOW);
  }
  prevX = x;
  prevY = y;
  prevZ = z;

  if ((tempX >= 80) || (tempX <= -80) || (tempY >= 80) || (tempY <= -80) || (tempZ >= 80) || (tempZ <= -80)) {
    /*Serial.println("Start");
    Serial.println(tempX - xPeaks[counter]);
    Serial.println(tempY - yPeaks[counter]);
    Serial.println(tempZ - zPeaks[counter]);
    Serial.println(counter);
    Serial.println("Finish");*/
    if ((((tempX - xPeaks[counter]) >= 15) || ((tempX - xPeaks[counter]) <= -15)) || (((tempY - yPeaks[counter]) >= 15) || ((tempY - yPeaks[counter]) <= -15)) || (((tempZ - zPeaks[counter]) >= 15) || ((tempZ - zPeaks[counter]) <= -15))) {
      order = true;
      counter++;
        
      if (counter < endP) {
        counter = 0;
        order = false;
        digitalWrite(buzzPin, LOW);
      }
    }
  }
}

bool isArmed() {
  if(armed) {
    if (!order) {
      counter = 0;
    }
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
        } else if (switchCharacteristic.value() == 2) {
          Serial.println("Setting user password");
          setPassword();
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
