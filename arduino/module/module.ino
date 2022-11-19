#include <Wire.h>
#include "packet.h"

#define UNASS_ADDR 0x08

#define RDYO_PIN 15
#define RDYA_PIN 13
#define RDYB_PIN 12
#define CONA_PIN 11
#define CONB_PIN 10

#define RED_LED_PIN 7
#define GREEN_LED_PIN 6

void(* resetFunc) (void) = 0;



// ===============================
//  Variables
// ===============================

byte addr = UNASS_ADDR;
Packet packet;
bool redOn = true;
bool greenOn = false;



// ===============================
// Handlers
// ===============================

void negotiateAddress() {
	byte dat = 0;

  if(Wire.available() == 1) {
		dat = Wire.read();
	}

  if(dat > UNASS_ADDR) addr = dat;
  else return;

	Wire.end();
	initI2C();
  setGreenLED();
}

void onReceive() {
	// Construct packet
	int idx = 0;
	byte buffer[MAX_PACKET_LEN];

	while(Wire.available()) {				// Read available data into buffer
		buffer[idx++] = Wire.read();
	}

	packet = Packet::deserialize(buffer);
  blinkGreenLED();
}

void onRequest() {
	// Interpret packet (See packet.h)
	if(packet.cmd == 0x01) enableSideA();
	if(packet.cmd == 0x02) enableSideB();
	if(packet.cmd == 0x05) resetFunc();

	packet.destroy();
}



// ===============================
// Functions
// ===============================

void initI2C() {
	Wire.begin(addr);
	Wire.onRequest(onRequest);
	if(addr == UNASS_ADDR) {
		Wire.onReceive(negotiateAddress);
	} else {
		Wire.onReceive(onReceive);
	}
}

void enableSideA() {
	digitalWrite(RDYA_PIN, HIGH);
  delay(10);
	byte connected = digitalRead(CONA_PIN) & 0xFF;

  if (connected > 0) blinkGreenLED();
  else blinkRedLED();

	Wire.write(connected);
}

void enableSideB() {
	digitalWrite(RDYB_PIN, HIGH);
  delay(10);
	byte connected =  digitalRead(CONB_PIN) & 0xFF;

  if (connected > 0) blinkGreenLED();
  else blinkRedLED();

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

void blinkRedLED() {
	digitalWrite(RED_LED_PIN, redOn ? LOW : HIGH);
  delay(50);
	digitalWrite(RED_LED_PIN, redOn ? HIGH : LOW);
}

void blinkGreenLED() {
	digitalWrite(GREEN_LED_PIN, greenOn ? LOW : HIGH);
  delay(50);
	digitalWrite(GREEN_LED_PIN, greenOn ? HIGH : LOW);
}

void initPins() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  pinMode(RDYO_PIN, INPUT);  
  pinMode(RDYA_PIN, OUTPUT);  
  pinMode(RDYB_PIN, OUTPUT);  
  pinMode(CONA_PIN, INPUT);  
  pinMode(CONB_PIN, INPUT);  

  digitalWrite(RDYA_PIN, LOW);
  digitalWrite(RDYB_PIN, LOW);
}



// ===============================
// Main Functions
// ===============================

void setup() {
  addr = UNASS_ADDR;
  initPins();
	setRedLED();
  
	while (digitalRead(RDYO_PIN) == LOW) delay(1);
  blinkRedLED();
  
	initI2C();
  blinkGreenLED();
}

void loop() {
  
}