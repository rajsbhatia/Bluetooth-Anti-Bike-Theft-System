#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service "19b10000e8f2537e4f6cd104768a1214"

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED
const int buzzPin = 8;

// gryoscope values
float x, y, z;
float prevX, prevY, prevZ;

bool armed = false;

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

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

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

void readGyro() {
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
  }

  bool check = true;
  int tempDiff = 0;
  for (int i = 0; i < 3; i++){
    if (i == 0){
      tempDiff = prevX - x;
    }
    else if (i == 1){
      tempDiff = prevY - y;
    }
    else if (i == 2){
      tempDiff = prevZ - z;
    }
    
    if ((tempDiff >= 15) || (tempDiff <= -15)){
      digitalWrite(buzzPin, HIGH);
      check = false;
      Serial.println("******start*****");
      Serial.println(x);
      Serial.println(y);
      Serial.println(z);
      Serial.println("******middle*****");
      Serial.println(prevX);
      Serial.println(prevY);
      Serial.println(prevZ);
      Serial.println("******end*****");
    }
  }
  
  if (check) {
    digitalWrite(buzzPin, LOW);
  }
  prevX = x;
  prevY = y;
  prevZ = z;
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
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
          armed = true;
        } else {                              // a 0 value
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
