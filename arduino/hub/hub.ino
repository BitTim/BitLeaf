#include <Wire.h>
#include "packet.h"

#define UNASS_ADDR 0x08
#define MIN_ADDR 0x09
#define MAX_ADDR 0x77

#define MAX_PANELS 32
#define TIMEOUT 3000

#define RDY_PIN 8
#define CON_PIN 7

void(* resetFunc) (void) = 0;



// ===============================
// Structs
// ===============================

struct Panel {
	byte addr;
	byte origin_addr;
	byte sideA_addr;
	byte sideB_addr;
};



// ===============================
// Vars
// ===============================

Panel panels[MAX_PANELS] = { {0, 0, 0, 0} };
bool usedAddr[MAX_PANELS] = { false };




// ===============================
// Handlers
// ===============================

// PLACEHOLDER



// ===============================
// Functions
// ===============================

byte getNextAddress() {
	byte addr = 0;
	for(int i = 0; i < MAX_PANELS; i++) {
		if(usedAddr[i] == false) {
			usedAddr[i] = true;
			addr = idxToAddr(i);
		}
	}

	return addr;
}

int addrToIdx(int addr) {
	return addr - MIN_ADDR;
}

int idxToAddr(int addr) {
	return addr + MIN_ADDR;
}

void registerPanel(byte side_addr, byte origin_addr, bool sideA) {
	byte side_addr = checkUnassigned();									// Get new address

	if(sideA) panels[addrToIdx(origin_addr)].sideA_addr = side_addr;	// Set variables for origin panel
	else panels[addrToIdx(origin_addr)].sideB_addr = side_addr;

	panels[addrToIdx(side_addr)].addr = side_addr;						// Set variables for current panel
	panels[addrToIdx(side_addr)].origin_addr = origin_addr;

	addresses[addrToIdx(side_addr)]
}

void unregisterPanel(byte side_addr, byte origin_addr, bool sideA) {
	if(panels[addrToIdx(side_addr)].sideA_addr != 0) {
		unregisterPanel(panels[addrToIdx(side_addr)].sideA_addr, side_addr, true);
	}

	if(panels[addrToIdx(side_addr)].sideB_addr != 0) {
		unregisterPanel(panels[addrToIdx(side_addr)].sideB_addr, side_addr, false);
	}

	usedAddr[addToIdx(side_addr)] = false;						// Mark address as free

	panels[addrToIdx(side_addr)].addr = 0;						// Reset variables of disconnected panel
	panels[addrToIdx(side_addr)].origin_addr = 0;

	if(sideA) panels[addrToIdx(origin_addr)].sideA_addr = 0;	// Reset variables for current panel
	else panels[addrToIdx(origin_addr)].sideB_addr = 0;
}

void checkSide(byte addr, bool sideA) { 						// If sideA is false, side B will be used
  	byte error = checkAvailable(addr);							// Check if addr is available
  	if(error != 0) {
		Serial.print("Panel ");
		Serial.print(addr);
		Serial.print(" does not respond, error: ");
		Serial.println(error);

		return;
	}

	// Send request to discover side A
	Wire.beginTransmission(addr);
  	byte packet[2] = { sideA ? Commands::sideA : Commands::sideB, 0 };
	Wire.write(packet, 2);
	Wire.endTransmission();

	Serial.print("Checking side ");
	Serial.print(sideA ? "A" : "B");
	Serial.print(" of panel ");
	Serial.println(addr);

	// Request response
	byte ret = 0;
	int len = Wire.requestFrom(addr, 1);
	if (len == 0) {
		Serial.print("Panel ");
		Serial.print(addr);
		Serial.println(" did not respond");
	} else {
		int timeout = 0;
		while(Wire.available() < 1 && timeout++ < TIMEOUT) { delay(1); }
	  	if(Wire.available() > 0) ret = Wire.read();
  	}

  	Serial.println(ret == 1 ? "Connected" : "Not connected");

	byte side_addr = 0;
	if(sideA) byte side_addr = panels[addrToIdx(addr)].sideA_addr;
	else byte side_addr = panels[addrToIdx(addr)].sideB_addr;

	if(ret == 1) {												// If a panel is connected to Side
		if (side_addr == 0) {
			registerPanel(origin_addr, sideA);
		}

		discoverPanel(side_addr);								// Recursivity
	} else {
		if (side_addr != 0) {
			unregisterPanel(side_addr, origin_addr, sideA);
		}
	}
}

void discoverPanel(int addr) {
	checkSide(addr, true);
	checkSide(addr, false);
}

void startDiscovery() {
	// Enable first panel
	if(digitalRead(CON_PIN) == LOW) { return; } // No panel connected to HUB
	digitalWrite(RDY_PIN, HIGH);

	byte addr = checkUnassigned();
  	if(addr == 0) return;

	panels[0].addr = addr;
	panels[0].origin_addr = 1; // 1 represents Hub

	// Recursively discover connected panels
	discoverPanel(addr);
}

byte assignAddress() {
	byte addr = getNextAddress();

	Serial.print("Assigning address ");
	Serial.print(addr);
	Serial.println(" to new panel");

	Wire.beginTransmission(UNASS_ADDR);
	Wire.write(addr);
	Wire.endTransmission();

  	byte ret = 0;
	Wire.requestFrom(UNASS_ADDR, 0);

	return addr;
}

byte checkAvailable(byte addr) {
	int timeout = 0;
	byte error = 1;

	Serial.print("Checking availability of panel ");
	Serial.println(addr);  

	while(error != 0 && timeout++ < TIMEOUT) {
		Wire.beginTransmission(addr);
		error = Wire.endTransmission();
		delay(1);
	}

	Serial.print("Result: ");
	Serial.println(error);
	return error;
}

byte checkUnassigned() {
	byte error = checkAvailable(UNASS_ADDR);
	if(error != 0) {
		Serial.println("No unassigned devices");
		return 0;
	}

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

void initPins() {
	pinMode(RDY_PIN, OUTPUT);
	pinMode(CON_PIN, INPUT);

	digitalWrite(SDA, LOW);
	digitalWrite(SCL, LOW);
}



// ===============================
// Main Functions
// ===============================

void setup() {
  	initPins();
  
	Serial.begin(9600);
	Wire.begin();
	Wire.setWireTimeout(TIMEOUT, false);

	startDiscovery();

	Serial.println("Finished discovery");
	Serial.println("");
	
	for(int i = 0; i < MAX_PANELS; i++) {
		if(panels[i].addr == 0) continue;

		Serial.println("----------------");
		
			Serial.print("Panel Address: ");
		Serial.println(panels[i].addr);

		Serial.print("Origin: ");
		Serial.println(panels[i].origin_addr);

		Serial.print("Side A: ");
		Serial.println(panels[i].sideA_addr);
		
		Serial.print("Side B: ");
		Serial.println(panels[i].sideB_addr);
	}
}

void loop() {
	
}