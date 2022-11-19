#include <Wire.h>
#include "../lib/packet.h"
#include "../lib/panel.h"

#define UNASS_ADDR 0x08
#define MIN_ADDR 0x09
#define MAX_ADDR 0x77

#define MAX_PANELS 32

#define RDY_PIN 8
#define CON_PIN 7



// ===============================
// Vars
// ===============================

Panel[MAX_PANELS] panels;
int lastAddr = MIN_ADDR;




// ===============================
// Handlers
// ===============================

void onReceive() {

}



// ===============================
// Functions
// ===============================

int getNextAddress() {
	//TODO: If gaps in the address space exist, fill those first
	return ++lastAddr;
}

int addrToIdx(int addr) {
	return addr - MIN_ADDR;
}

void discoverPanel(int addr) {
	// Send request to discover side A
	Wire.startTransmission(addr);
	Wire.write(Packet::eSideAReq);
	Wire.endTransmission();

	// Request response
	byte ret = 0;
	Wire.requestFrom(addr, 1)
	if(Wire.available()) ret = Wire.read();

	if(ret == 1) {											// If a panel is connected to Side A
		byte sideA_addr = checkUnassigned();				// Assign and retrieve address for new panel
		panels[addrToIdx(addr)].sideA_addr = sideA_addr;	// Set variables for tree structure
		panels[addrToIdx(sideA_addr)].addr = sideA_addr;
		panels[addrToIdx(sideA_addr)].origin_addr = addr;

		discoverPanel(sideA_addr);							// Recursivity
	} else {
		panels[addrToIdx(addr)].sideA_addr = 0;				// No panel connected to side A
	}

	// Repeat with side B
	Wire.startTransmission(addr);
	Wire.write(Packet::eSideBReq);
	Wire.endTransmission();

	// Request response
	byte ret = 0;
	Wire.requestFrom(addr, 1)
	if(Wire.available()) ret = Wire.read();

	// If a panel is connected to Side B
	if(ret == 1) {
		byte sideB_addr = checkUnassigned();				// Assign and retrieve address for new panel
		panels[addrToIdx(addr)].sideB_addr = sideB_addr;	// Set variables for tree structure
		panels[addrToIdx(sideB_addr)].addr = sideB_addr;
		panels[addrToIdx(sideB_addr)].origin_addr = addr;

		discoverPanel(sideB_addr);							// Recursivity
	} else {
		panels[addrToIdx(addr)].sideB_addr = 0;				// No panel connected to side B
	}
}

void startDiscovery() {
	// Enable first panel
	digitalWrite(RDY_PIN, HIGH);
	if(digitalRead(CON_PIN) == LOW) {
		// No panel connected to HUB
		return;
	}

	byte addr = getNextAddress();
	panels[0].addr = addr;
	panels[0].origin_addr = 1; // 1 represents Hub

	// Recursively discover connected panels
	discoverPanel(addr);
}

byte assignAddress() {
	byte addr = getNextAddress();

	Wire.beginTransmission(UNASS_ADDR);
	Wire.write(addr);
	Wire.endTransmission();

	return addr;
}

byte checkUnassigned() {
	Wire.beginTransmission(UNASS_ADDR);
	byte error = Wire.endTransmission();

	if(error != 0) return 0;
	byte addr = assignAddress();
	return addr;
}

void hotplug() {
	// Hotplug support:
	// Enable all sides with packets
	// When new module connected to already enabled side, interrupt
	// Interrupted module sends address to hub
	// Hub can then assign the origin address for new module and put it into tree structure 
	// Same applies for disconnecting
}



// ===============================
// Main Functions
// ===============================

void start() {
	Wire.begin();
	startDiscovery();

	Serial.begin(9600);
	for(int i = 0; i < MAX_PANELS) {
		
	}
}

void loop() {
	// checkUnassigned();
	// delay(10);
}