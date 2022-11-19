#ifndef PACKET_H
#define PACKET_H

#define MAX_PACKET_LEN 16

// ================================================================
//  Packet commands
// ----------------------------------------------------------------
//  0x00		NOP							[]
//	0x01		Enable side A				[]
//  0x02		Enable side B				[]
//	0x03		Disable side A				[]
//  0x04		Disable side B				[]
//  0x05    Reset Panel           []
//	0x06		Set LED Color				[LED_IDX, R, G, B]
//	0x07		Set LED Brightness			[LED_IDX, BRIGHTNESS]
//	0x08		Set Panel Color				[R, G, B]
//	0x09		Set Panel Brightness		[BRIGHTNESS]
// ================================================================

class Packet
{
public:
	byte cmd;
	byte len;
	byte* dat;

  Packet() {}

	Packet(byte _cmd, byte _len, byte* _dat) {
		cmd = _cmd;
		len = _len;
		dat = _dat;
	}

	byte* serialize() {
		byte* serialized = malloc(len + 2);
		serialized[0] = cmd;
		serialized[1] = len;
		memcpy(serialized + 2, dat, len);

		return serialized;
	}

	static Packet deserialize(byte* serialized) {
		byte cmd = serialized[0];
		byte len = serialized[1];

		if(MAX_PACKET_LEN < len + 2) len = MAX_PACKET_LEN - 2;

		byte* dat;
		memcpy(dat, serialized + 2, len);
		free(serialized);

		return Packet(cmd, len, dat);
	}

	void destroy() {
		cmd = 0;
		len = 0;
		free(dat);
	}



  // ================================
  //  Presets
  // ================================

  static Packet eSideA() { return Packet(0x01, 0, nullptr); }     
  static Packet eSideB() { return Packet(0x02, 0, nullptr); }     
  static Packet dSideA() { return Packet(0x03, 0, nullptr); }     
  static Packet dSideB() { return Packet(0x04, 0, nullptr); }     
  static Packet reset()  { return Packet(0x05, 0, nullptr); }     
};

#endif // PACKET_H