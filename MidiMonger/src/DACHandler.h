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

enum DacCommand : uint8_t {WriteChannel = 0b011, Reset = 0b101, InternalRef = 0b111};
enum DacAddress : uint8_t{ChannelA = 0, ChannelB = 1, ChannelC = 2, ChannelD = 3, AllChannels = 7};

class DACHandler
{
public:
	void initDAC();
	void sendData(DacCommand, DacAddress, uint16_t data);
};
