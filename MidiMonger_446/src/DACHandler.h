#pragma once

#include "initialisation.h"

union DacData {
	struct {
		uint8_t addr: 3, cmd: 5;
		uint16_t data;
	};
	uint8_t da[3];
};

#define hardwareNSS 0
#define MAX5134 1

#ifdef MAX5134
enum DacCommand : uint8_t {WriteChannel = 0b0011 << 4, Clear = 0b0010, Linearity = 0b101};
enum DacAddress : uint8_t {ChannelA = 1, ChannelB = 2, ChannelC = 4, ChannelD = 8};
#else
enum DacCommand : uint8_t {WriteChannel = 0b011, Reset = 0b101, InternalRef = 0b111};
enum DacAddress : uint8_t{ChannelA = 0, ChannelB = 1, ChannelC = 2, ChannelD = 3, AllChannels = 7};
#endif

class DACHandler
{
public:
	void initDAC();
#ifdef MAX5134
	void sendData(uint8_t cmd, uint16_t data);
#else
	//void sendData(DacCommand, DacAddress, uint16_t data);
#endif
};
