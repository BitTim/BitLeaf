#ifndef PACKET_H
#define PACKET_H

#define MAX_PACKET_LEN 32

// ================================================================
//  Packet commands
// ----------------------------------------------------------------
//  0x00		NOP							[]
//	0x01		Check side A				[]
//  0x02		Check side B				[]
//  0x05    	Reset Panel           		[]
//	0x06		Set LED Color				[LED_IDX, R, G, B]
//	0x07		Set LED Brightness			[LED_IDX, BRIGHTNESS]
//	0x08		Set Panel Color				[R, G, B]
//	0x09		Set Panel Brightness		[BRIGHTNESS]
// ================================================================

struct Commands {
	enum Commands_t {
		NOP,
		sideA,
		sideB,
		reset,
		ledCol,
		ledBright,
		panelCol,
		panelBright,
    toggleLED,
	};
};

#endif // PACKET_H