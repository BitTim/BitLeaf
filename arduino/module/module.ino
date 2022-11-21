#include <Wire.h>
#include "packet.h"

#define UNASS_ADDR 0x08

#define TIMEOUT 3000

#define RDYA_PIN 4
#define RDYB_PIN 5
#define CONA_PIN 6
#define CONB_PIN 7

#define RED_LED_PIN 8
#define YELLOW_LED_PIN 9
#define GREEN_LED_PIN 10

void(* resetFunc) (void) = 0;



// ===============================
//  Variables
// ===============================

volatile byte addr = UNASS_ADDR;
volatile byte recAddr = 0;
volatile byte inBuffer[MAX_PACKET_LEN] = { 0 };
bool redOn = true;
bool greenOn = false;



// ===============================
// Handlers
// ===============================

// TODO: save data from negotiate address and handle in separate onrequest handler which sends callback to hub

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
  
	initI2C();
  setGreenLED();
}

void onReceive() {
	// Construct packet
	int idx = 0;
  
  int timeout = 0;
  while(Wire.available() < 1 && timeout++ < TIMEOUT) { delay(1); }

	while(Wire.available() > 0) {				// Read available data into buffer
		inBuffer[idx++] = Wire.read();
    if(idx >= MAX_PACKET_LEN) break;
	}
}

void onRequest() {
	// Interpret packet (See packet.h)
	if(inBuffer[0] == Commands::eSideA) enableSide(true);
	if(inBuffer[0] == Commands::eSideB) enableSide(false);
	if(inBuffer[0] == Commands::reset)  reset();

	memset(inBuffer, 0, MAX_PACKET_LEN);
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