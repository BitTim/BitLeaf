#include <Wire.h>
#include "packet.h"
#include "panel.h"

#define UNASS_ADDR 0x08
#define MIN_ADDR 0x09
#define MAX_ADDR 0x77

#define MAX_PANELS 32

#define RDY_PIN 8
#define CON_PIN 7

void(* resetFunc) (void) = 0;
// TODO: replace delays with timeouts



// ===============================
// Vars
// ===============================

Panel panels[MAX_PANELS];
bool usedAddr[MAX_PANELS] = { false };
int lastAddr = MIN_ADDR;




// ===============================
// Handlers
// ===============================

void onReceive() {

}



// ===============================
// Functions
// ===============================

byte getNextAddress() {
	//TODO: If gaps in the address space exist, fill those first

  byte addr = lastAddr++;
  usedAddr[addrToIdx(addr)] = true;
	return addr;
}

int addrToIdx(int addr) {
	return addr - MIN_ADDR;
}

int idxToAddr(int addr) {
	return addr + MIN_ADDR;
}

void resetAllPanels() {
  digitalWrite(RDY_PIN, LOW);

  for(int i = 0; i < MAX_PANELS; i++) {
    byte addr = idxToAddr(i);
    Wire.beginTransmission(addr);
    int error = Wire.endTransmission();

    if (error != 0) continue;

    Wire.beginTransmission(addr);
    Packet packet = Packet::reset();
    Wire.write(packet.serialize(), sizeof(packet));
    Wire.endTransmission();

    Wire.requestFrom(addr, 0);
    delay(10);
  }

  delay(100);
}

void discoverPanel(int addr) {
	// Send request to discover side A
	Wire.beginTransmission(addr);
  Packet eSideA = Packet::eSideA();
	Wire.write(eSideA.serialize(), sizeof(eSideA));
	Wire.endTransmission();

  Serial.print("Checked side A of ");
  Serial.println(addr);

	// Request response
	byte ret = 0;
	Wire.requestFrom(addr, 1);
  int timeout = 0;
  while(Wire.available() < 1 && timeout++ < 30) { delay(10); }
	if(Wire.available()) ret = Wire.read();

  Serial.print("Connection state: ");
  Serial.println(ret == 1 ? "true" : "false");
  delay(100);

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
	Wire.beginTransmission(addr);
  Packet eSideB = Packet::eSideB();
	Wire.write(eSideB.serialize(), sizeof(eSideB));
	Wire.endTransmission();

  Serial.print("Checked side B of ");
  Serial.println(addr);

	// Request response
	ret = 0;
	Wire.requestFrom(addr, 1);
  timeout = 0;
  while(Wire.available() < 1 && timeout++ < 30) { delay(10); }
	if(Wire.available()) ret = Wire.read();

  Serial.print("Connection state: ");
  Serial.println(ret == 1 ? "true" : "false");
  delay(100);

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
  delay(10);
	if(digitalRead(CON_PIN) == LOW) { return; } // No panel connected to HUB
  delay(100);

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

void initPins() {
  pinMode(RDY_PIN, OUTPUT);
  pinMode(CON_PIN, INPUT);
}



// ===============================
// Main Functions
// ===============================

void setup() {
  initPins();
  
	Serial.begin(9600);
	Wire.begin();
  resetAllPanels();
	startDiscovery();

  Serial.println("");
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
	// checkUnassigned();
	// delay(10);
}