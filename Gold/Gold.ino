#include <SoftWire.h>
#include <AsyncDelay.h>

uint8_t sdaPin = 0;
uint8_t sclPin = 1;

int16_t axisX, axisY, axisZ;

int minVal = 265;
int maxVal = 402;

double x = 0.0;
double y = 0.0;
double z = 0.0;

SoftWire sw(sdaPin, sclPin);

void setup(void)
{

	Serial.begin(9600);

  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(22, OUTPUT);   

	//sw.setTimeout_ms(40);
  sw.setDelay_us(5);
	sw.begin();
  delay(1000);

  SoftWire::sclLow(&sw);
  delay(3); // Must be > 1.44ms
  SoftWire::sclHigh(&sw);
  delay(2);

	// Set how long we are willing to wait for a device to respond
	//sw.setTimeout_ms(200);

	const uint8_t firstAddr = 1;
	const uint8_t lastAddr = 0x7F;
	Serial.println();
	Serial.print("Interrogating all addresses in range 0x");
	Serial.print(firstAddr, HEX);
	Serial.print(" - 0x");
	Serial.print(lastAddr, HEX);
	Serial.println(" (inclusive) ...");

	for (uint8_t addr = firstAddr; addr <= lastAddr; addr++) {
		//digitalWrite(LED_BUILTIN, HIGH);
		delayMicroseconds(50);
 
		uint8_t startResult = sw.llStart((addr << 1) + 1); // Signal a read
		sw.stop();

		if (startResult == 0) {
			Serial.print("\rDevice found at 0x");
      digitalWrite(22, HIGH);
			Serial.println(addr, HEX);
			Serial.flush();
		}

		delay(50);
	}
	Serial.println("Finished");

}

void loop(void)
{
  uint8_t low = 0;
  uint8_t high = 0;
  uint8_t pec = 0;
  int crc = 0;
  
  sw.startWait(0x68, SoftWire::writeMode);
  sw.write(0x6B);
  delay(500);
  sw.write(0x3B);

  sw.repeatedStart(0x68, SoftWire::readMode);
  for(int i = 0; i < 3; i++){
    
  
    sw.readThenAck(low);
    sw.readThenAck(high);

    if (i == 0){
      axisX = high<<8|low;
    }
    if (i == 1){
      axisY = high<<8|low;
    }
    if (i == 2){
      axisZ = high<<8|low;
    }

    int xAng = map(axisX, minVal, maxVal, -90, 90);
    int yAng = map(axisY, minVal, maxVal, -90, 90);
    int zAng = map(axisZ, minVal, maxVal, -90, 90);

    x = RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
    y = RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
    z = RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
  }
  sw.readThenNack(pec);
  sw.stop();

  crc = 0;
  crc = SoftWire::crc8_update(crc, 0x68 << 1); // Write address
  //crc = SoftWire::crc8_update(crc, command);
  crc = SoftWire::crc8_update(crc, (0x68 << 1) + 1); // Read address
  crc = SoftWire::crc8_update(crc, x);
  crc = SoftWire::crc8_update(crc, y);
  crc = SoftWire::crc8_update(crc, z);
  crc = SoftWire::crc8_update(pec, pec);

  Serial.println("***********");
  Serial.println(x);
  Serial.println(y);
  Serial.println(z);
  delay(1000);

  if (x<0){
    digitalWrite(18, HIGH);
  }
  else if (x>0){
    digitalWrite(19, HIGH);
  }
  delay(1000);
  digitalWrite(18, LOW);
  digitalWrite(19, LOW);
}
