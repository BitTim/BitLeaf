#include <Wire.h>
#include "packet.h"

#define UNASS_ADDR 0x08
#define MIN_ADDR 0x09
#define MAX_ADDR 0x77

#define MAX_PANELS 32
#define TIMEOUT 3000
#define FPS 48

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
bool panelsChanged = false;
bool usedAddr[MAX_PANELS] = { false };
int connectedPanels = 0;

int frame = 0;



// ===============================
// Handlers
// ===============================

// PLACEHOLDER



// ===============================
// Protocol Functions
// ===============================

byte getNextAddress() {
	byte addr = 0;
	for(int i = 0; i < MAX_PANELS; i++) {
		if(usedAddr[i] == false) {
			usedAddr[i] = true;
			addr = idxToAddr(i);
      break;
		}
	}

	return addr;
}

int addrToIdx(int addr) {
	return addr - MIN_ADDR;
}

int idxToAddr(int idx) {
	return idx + MIN_ADDR;
}

void registerPanel(byte origin_addr, bool sideA) {
	byte side_addr = checkUnassigned();									// Get new address

	if(sideA) panels[addrToIdx(origin_addr)].sideA_addr = side_addr;	// Set variables for origin panel
	else panels[addrToIdx(origin_addr)].sideB_addr = side_addr;

	panels[addrToIdx(side_addr)].addr = side_addr;						// Set variables for current panel
	panels[addrToIdx(side_addr)].origin_addr = origin_addr;

	usedAddr[addrToIdx(side_addr)];
  panelsChanged = true;
  connectedPanels++;
}

void unregisterPanel(byte side_addr, byte origin_addr, bool sideA) {
	if(panels[addrToIdx(side_addr)].sideA_addr != 0) {
		unregisterPanel(panels[addrToIdx(side_addr)].sideA_addr, side_addr, true);
	}

	if(panels[addrToIdx(side_addr)].sideB_addr != 0) {
		unregisterPanel(panels[addrToIdx(side_addr)].sideB_addr, side_addr, false);
	}

	usedAddr[addrToIdx(side_addr)] = false;							// Mark address as free

	panels[addrToIdx(side_addr)].addr = 0;							// Reset variables of disconnected panel
	panels[addrToIdx(side_addr)].origin_addr = 0;

	if(origin_addr >= MIN_ADDR) {
		if(sideA) panels[addrToIdx(origin_addr)].sideA_addr = 0;	// Reset variables for current panel
		else panels[addrToIdx(origin_addr)].sideB_addr = 0;
	}

  panelsChanged = true;
  connectedPanels--;
}

void checkSide(byte addr, bool sideA) { 							// If sideA is false, side B will be used
  byte error = checkAvailable(addr);								  // Check if addr is available
  if(error != 0) { return; }

	// Send request to discover side A
  byte packet[2] = { sideA ? Commands::sideA : Commands::sideB, 0 };
	Wire.beginTransmission(addr);
	Wire.write(packet, 2);
	Wire.endTransmission();

	// Request response
	byte ret = 0;
	int len = Wire.requestFrom(addr, 1);
	if (len > 0) {
		int timeout = 0;
		while(Wire.available() < 1 && timeout++ < TIMEOUT) { delay(1); }
    if(Wire.available() > 0) ret = Wire.read();
  }

	byte side_addr = 0;
	if(sideA) side_addr = panels[addrToIdx(addr)].sideA_addr;
	else side_addr = panels[addrToIdx(addr)].sideB_addr;

	if(ret == 1) {												// If a panel is connected to Side
		if (side_addr == 0) {
			registerPanel(addr, sideA);
		} else {
		  discoverPanel(side_addr);								// Recursivity
    }      
	} else {
		if (side_addr != 0) {
			unregisterPanel(side_addr, addr, sideA);
		}
	}
}

void discoverPanel(int addr) {
	checkSide(addr, true);
	checkSide(addr, false);
}

void startDiscovery() {
	if(digitalRead(CON_PIN) == LOW) {  										            // No panel connected to HUB
		if(panels[addrToIdx(MIN_ADDR)].addr != 0) {
      unregisterPanel(MIN_ADDR, 1, true);
    }
		return;
	}

	if(panels[addrToIdx(MIN_ADDR)].addr == 0) {												// New panel connected and has no addr
		byte addr = checkUnassigned();
  	if(addr == 0) return;

		panels[addrToIdx(MIN_ADDR)].addr = addr;
		panels[addrToIdx(MIN_ADDR)].origin_addr = 1;                    // 1 represents Hub

    connectedPanels++;
	}

	discoverPanel(MIN_ADDR);	                                        // Recursively discover connected panels
}

byte assignAddress() {
	byte addr = getNextAddress();

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

	while(error != 0 && timeout++ < TIMEOUT) {
		Wire.beginTransmission(addr);
		error = Wire.endTransmission();
		delay(1);
	}
	return error;
}

byte checkUnassigned() {
	byte error = checkAvailable(UNASS_ADDR);
	if(error != 0) { return 0; }

	byte addr = assignAddress();
	return addr;
}



// ===============================
// Animation Functions
// ===============================

void advanceAnimation() {           // Sample animation for testing
  int frameMultiplyer = floor((double) FPS / (double) connectedPanels);

  if(frame % frameMultiplyer == 0) {
    byte addr = idxToAddr(frame / frameMultiplyer);
    
    Serial.println(frame);
    Serial.println(frameMultiplyer);
    Serial.println(addr);
    Serial.println();

    if(checkAvailable(addr) != 0) return;

    byte packet[2] = {Commands::toggleLED, 0};
    Wire.beginTransmission(addr);
    Wire.write(packet, 2);
    Wire.endTransmission();

    Wire.requestFrom(addr, 0);
  }
  
  frame++;
  if(frame >= FPS) frame = 0;
}



// ===============================
// Util Functions
// ===============================

void initPins() {
	pinMode(RDY_PIN, OUTPUT);
	pinMode(CON_PIN, INPUT);

	digitalWrite(SDA, LOW);
	digitalWrite(SCL, LOW);

	digitalWrite(RDY_PIN, HIGH);
}



// ===============================
// Main Functions
// ===============================

void setup() {
  initPins();
  
	Serial.begin(9600);
	Wire.begin();
	Wire.setWireTimeout(TIMEOUT, false);
}

void loop() {
	startDiscovery();
  
  if(panelsChanged) {
    panelsChanged = false;

	  for(int i = 0; i < MAX_PANELS; i++) {
		  if(panels[i].addr == 0) continue;
		
			Serial.print("Panel Address: ");
      Serial.println(panels[i].addr);

      Serial.print("Origin: ");
      Serial.println(panels[i].origin_addr);

      Serial.print("Side A: ");
      Serial.println(panels[i].sideA_addr);
      
      Serial.print("Side B: ");
      Serial.println(panels[i].sideB_addr);

      Serial.println("----------------");
    }

    Serial.println("================");
    Serial.println("");
  }
	
  advanceAnimation();
	delay(1000 / FPS);
}