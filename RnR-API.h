#pragma once

#pragma pack(push, 1)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;


struct RnRUSBBuff
{
//	CUSBBuff() : ReportID(0), UpdateFlag(0), Data(0), ID(0), LED(0), DisplayMode(0), Uptime(0) ,/* Reserved(0) ,*/ WDTime(0), WDTimeLeft(0), Data2(0) {}
	const char ReportID;
	uint8_t UpdateFlag;		// Write option bits
							//	1  - Relay value;
							//	2  - Storing ID value
							//	4  - Selecting display mode
							//	8  - WDT preset
							//	16 - WDT restart using current preset
							//	 Note that any option bits combination is acceptable
							//	 0 value has no effect for device !!!
							// For read operation always equal to 0
	uint8_t Data;			// Relays state for "write" operation, Rays state for "read" operation. Bits 0..3 are used.
	uint8_t ID;				// internal device id [0..255]
	uint8_t LED;			// according to DisplayMode shows device id [0..15] - hi nibble for flashing (each LED separately)
	uint8_t DisplayMode;	//	0 - LED indicator displays LED (see below) value
							//	1 - Relays states
							//	2 - Rays states
							//	3 - activity on watchdog timer, R'n'R pins and USB interface
							//	4 - internal state
							//	5 - internal state
	uint32_t Uptime;		// Count of sec since device rising
	uint16_t WDTime;		// Watchdog timer presetting value (sec)
	uint16_t WDTimeLeft;	// Watchdog timer countdown (sec)
	uint8_t  Data2;			// Relays state for "read" operation. Bits 0..3 are used.
//	uint8_t  Reserved;
};
typedef struct RnRUSBBuff RnRUSBBuff;
#pragma pack(pop)

