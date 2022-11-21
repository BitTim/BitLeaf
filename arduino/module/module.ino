#include <Wire.h>
#include "packet.h"

#define UNASS_ADDR 0x08

#define TIMEOUT 3000

#define RDYA_PIN 13
#define RDYB_PIN 12
#define CONA_PIN 11
#define CONB_PIN 10

#define RED_LED_PIN 7
#define YELLOW_LED_PIN 6
#define GREEN_LED_PIN 5

void(* resetFunc) (void) = 0;



// ===============================
//  Variables
// ===============================

volatile byte addr = UNASS_ADDR;
volatile byte recAddr = 0;
volatile Packet packet;
bool redOn = true;
bool yellowOn = false;
bool greenOn = false;
volatile bool queueReset = false;



// ===============================
// Handlers
// ===============================

// TODO: save data from negotiate address and handle in separate onrequest handler which sends callback to hub

,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
++++++++++++++++++++++++++++
void negotiateAddress() {
  int timeout = 0;
  while(Wire.available() < 1 && timeout++ < TIMEOUT) { delay(1); }
  
  if(Wire.available() > 0) { recAddr = Wire.read(); }
}

void applyAddress()
{
  if(recAddr <= UNASS_ADDR) {
    reset();
    return;
  }

  addr = recAddr;

	Wire.end();
  delay(100);
	initI2C();
  setGreenLED();
}

void onReceive() {
	// Construct packet
	int idx = 0;
	byte buffer[MAX_PACKET_LEN];
  
  int timeout = 0;
  while(Wire.available() < 1 && timeout++ < TIMEOUT) { delay(1); }

	while(Wire.available() > 0) {				// Read available data into buffer
		buffer[idx++] = Wire.read();
	}

  packet.destroy();
	packet = Packet::deserialize(buffer);
}

void onRequest() {
	// Interpret packet (See packet.h)
	if(packet.cmd == 0x01) enableSide(true);
	if(packet.cmd == 0x02) enableSide(false);
	if(packet.cmd == 0x05) reset();

	packet.destroy();
}



// ===============================
// Functions
// ===============================

void reset()
{
  Wire.write(1);
  Wire.end();
  resetFunc();
}

void initI2C() {
  Wire.begin(addr);
	if(addr == UNASS_ADDR) {
		Wire.onReceive(negotiateAddress);
    Wire.onRequest(applyAddress);
	} else {
		Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);
	}
}

void enableSide(bool sideA) { // if sideA is false, side B will be used
	byte connected = digitalRead(sideA ? CONA_PIN : CONB_PIN) & 0xFF;

  if (connected > 0) {
	  digitalWrite(sideA ? RDYA_PIN : RDYB_PIN, HIGH);
  }

	Wire.write(connected);
}

void setRedLED() {
  redOn = true;
  greenOn = false;
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, LOW);
}

void setGreenLED() {
  redOn = false;
  greenOn = true;
	digitalWrite(RED_LED_PIN, LOW);
	digitalWrite(GREEN_LED_PIN, HIGH);
}

void initPins() {
  pinMode(RED_LED_PIN, OUTPUT);  
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  pinMode(RDYA_PIN, OUTPUT);  
  pinMode(RDYB_PIN, OUTPUT);  
  pinMode(CONA_PIN, INPUT);  
  pinMode(CONB_PIN, INPUT);  

  digitalWrite(RDYA_PIN, LOW);
  digitalWrite(RDYB_PIN, LOW);

  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
}



// ===============================
// Main Functions
// ===============================

void setup() {
  addr = UNASS_ADDR;
  initPins();
	setRedLED();
  
	initI2C();
}

void loop() {
  
}