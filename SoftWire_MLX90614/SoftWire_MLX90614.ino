#include <AsyncDelay.h>
#include <SoftWire.h>

/* SoftWire_MLX90614
 *
 * Code to demonstrate how to communicate with an SMBus device such as
 * the MLX90614 non-contact infrared thermometer.
 *
 * Connections:
 *
 * For Calunium it is assumed the JTAG pins are used, SDA pin = 16
 * (JTAG TDI) and SCL pin = 19 (JTAG TCK) and that the device is
 * powered from pin 18 (JTAG TMS).
 * For all other boards it is assumed that the MLX90614 is permanently
 * powered and that SDA = A4 and SCL = A5.
 *
 */


/*#ifdef CALUNIUM
#include <DisableJTAG.h>

uint8_t sdaPin = 16; // JTAG TDI
uint8_t sclPin = 19; // JTAG TCK
uint8_t powerPin = 18;
#elif defined(ARDUINO_ARCH_AVR)
uint8_t sdaPin = A4;
uint8_t sclPin = A5;
#else*/
// Adjust to suit your non-AVR architecture
uint8_t sdaPin = 0;
uint8_t sclPin = 1;
//#endif

/*const uint8_t cmdAmbient = 6;
const uint8_t cmdObject1 = 7;
const uint8_t cmdObject2 = 8;
const uint8_t cmdFlags = 0xf0;
const uint8_t cmdSleep = 0xff;
*/

SoftWire i2c(sdaPin, sclPin);

AsyncDelay samplingInterval;


uint16_t readMLX90614(uint8_t command, uint8_t &crc)
{
	uint8_t address = 0x68;
	uint8_t dataLow = 0;
	uint8_t dataHigh = 0;
	uint8_t pec = 0;

	uint8_t errors = 0;
	digitalWrite(LED_BUILTIN, HIGH); delayMicroseconds(50);
	// Send command
	//errors += i2c.startWait(address, SoftWire::writeMode);
	//errors += i2c.write(command);

	// Read results
	errors += i2c.startWait(address, SoftWire::readMode);
	errors += i2c.readThenAck(dataLow);  // Read 1 byte and then send ack
	errors += i2c.readThenAck(dataHigh); // Read 1 byte and then send ack
	errors += i2c.readThenNack(pec);
	i2c.stop();
	digitalWrite(LED_BUILTIN, LOW);

	crc = 0;
	crc = SoftWire::crc8_update(crc, address << 1); // Write address
	crc = SoftWire::crc8_update(crc, command);
	crc = SoftWire::crc8_update(crc, (address << 1) + 1); // Read address
	crc = SoftWire::crc8_update(crc, dataLow);
	crc = SoftWire::crc8_update(crc, dataHigh);
	crc = SoftWire::crc8_update(pec, pec);

	if (errors) {
		crc = 0xFF;
		return 0xFFFF;
	}
	return (uint16_t(dataHigh) << 8) | dataLow;
}




void setup(void)
{
	Serial.begin(9600);
	Serial.println("MLX90614_demo");

	pinMode(LED_BUILTIN, OUTPUT);

#ifdef CALUNIUM
	disableJTAG();
	pinMode(powerPin, OUTPUT);
	digitalWrite(powerPin, HIGH);
	Serial.println("Powering MLX90614");
	delay(1000);

#endif

	i2c.setDelay_us(5);
	i2c.begin();
	delay(300); // Data is available 0.25s after wakeup
}


void loop(void)
{
	digitalWrite(powerPin, HIGH);
	delay(300); // Data available after 0.25s
	exitPWM();
#endif

	uint8_t crcAmbient;
	uint16_t rawAmbient = readMLX90614(cmdAmbient, crcAmbient);
	uint8_t crcObject1;
	uint16_t rawObject1 = readMLX90614(cmdObject1, crcObject1);
	// Uncomment lines below for dual FoV sensors
	// uint8_t crcObject2;
	// uint16_t rawObject2 = readMLX90614(cmdObject2, crcObject2);

	
  Serial.println(rawAmbient);

	Serial.println();
	digitalWrite(powerPin, LOW);
	delay(1000);
}
