#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

long timer = 0;
#define LED_PIN_IN 20
#define LED_PIN_OUT 19

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  pinMode(LED_PIN_IN, OUTPUT);
  pinMode(LED_PIN_OUT, OUTPUT);
  pinMode(18, OUTPUT);

  pinMode(3, INPUT);
  pinMode(4, INPUT);

  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  
}

void loop() {
  mpu6050.update();

  //digitalWrite(LED_PIN_OUT, HIGH);

  if(millis() - timer > 1000){

    //digitalWrite(LED_PIN_IN, HIGH);

    if (mpu6050.getAccX() || mpu6050.getAccY() || mpu6050.getAccZ()){
      digitalWrite(20, HIGH);
      delay(500);
      digitalWrite(20, LOW);
    }

    if (mpu6050.getGyroX() || mpu6050.getGyroY() || mpu6050.getGyroZ()){
      digitalWrite(19, HIGH);
      delay(500);
      digitalWrite(19, LOW);
    }

    if (mpu6050.getAngleX() || mpu6050.getAngleY() || mpu6050.getAngleZ()){
      digitalWrite(18, HIGH);
      delay(500);
      digitalWrite(18, LOW);
    }

        
    Serial.println("=======================================================");
    Serial.print("temp : ");Serial.println(mpu6050.getTemp());
    Serial.print("accX : ");Serial.print(mpu6050.getAccX());
    Serial.print("\taccY : ");Serial.print(mpu6050.getAccY());
    Serial.print("\taccZ : ");Serial.println(mpu6050.getAccZ());
  
    Serial.print("gyroX : ");Serial.print(mpu6050.getGyroX());
    Serial.print("\tgyroY : ");Serial.print(mpu6050.getGyroY());
    Serial.print("\tgyroZ : ");Serial.println(mpu6050.getGyroZ());
  
    Serial.print("accAngleX : ");Serial.print(mpu6050.getAccAngleX());
    Serial.print("\taccAngleY : ");Serial.println(mpu6050.getAccAngleY());
  
    Serial.print("gyroAngleX : ");Serial.print(mpu6050.getGyroAngleX());
    Serial.print("\tgyroAngleY : ");Serial.print(mpu6050.getGyroAngleY());
    Serial.print("\tgyroAngleZ : ");Serial.println(mpu6050.getGyroAngleZ());
    
    Serial.print("angleX : ");Serial.print(mpu6050.getAngleX());
    Serial.print("\tangleY : ");Serial.print(mpu6050.getAngleY());
    Serial.print("\tangleZ : ");Serial.println(mpu6050.getAngleZ());
    Serial.println("=======================================================\n");
    timer = millis();

    //digitalWrite(LED_PIN_IN, LOW);
    
  }

  //digitalWrite(LED_PIN_OUT, LOW);

}
