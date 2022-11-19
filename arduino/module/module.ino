#include <Wire.h>
#include "../lib/packet.h"

#define UNASS_ADDR 0x08

#define RDYO_PIN 15
#define RDYA_PIN 13
#define RDYB_PIN 12
#define CONA_PIN 11
#define CONB_PIN 10

#define RED_LED_PIN 7
#define GREEN_LED_PIN 6



// ===============================
//  Variables
// ===============================

byte addr = UNASS_ADDR;
Packet packet;



// ===============================
// Handlers
// ===============================

void negotiateAddress() {
	if(Wire.available() == 1) {
		addr = Wire.read()
	}

	Wire.end();
	initI2C();
}

void onReceive() {
	// Construct packet
	int idx = 0;
	byte[MAX_PACKET_LEN] buffer;

	while(Wire.available()) {				// Read available data into buffer
		buffer[idx++] = Wire.read();
	}

	packet = Packet.deserialize(buffer);
}

void onRequest() {
	// Interpret packet (See packet.h)
	if(p.cmd == 0x02) enableSideA();
	if(p.cmd == 0x03) enableSideB();

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
	byte connected = digitalRead(CONA_PIN);

	Wire.write(connected);
}

void enableSideB() {
	digitalWrite(RDYB_PIN, HIGH);
	byte connected =  digitalRead(CONB_PIN);

	Wire.write(connected);
}

void setRedLED() {
	digitalWrite(RED_LED_PIN, HIGH);
	digitalWrite(GREEN_LED_PIN, LOW);
}

void setGreenLED() {
	digitalWrite(RED_LED_PIN, LOW);
	digitalWrite(GREEN_LED_PIN, HIGH);
}



// ===============================
// Main Functions
// ===============================

void start() {
	setRedLED();
	while (digitalRead(RDYO_PIN) == LOW) delay(10);
	initI2C();
	setGreenLED();
}

void loop() {

}