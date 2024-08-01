#pragma once

#include "initialisation.h"

union DacData {
	struct {
		uint8_t addr: 3, cmd: 5;
		uint16_t data;
	};
	uint8_t da[3];
};

enum DacCommand : uint8_t {WriteChannel = 0b0011 << 4, Clear = 0b0010, Linearity = 0b101};
enum DacAddress : uint8_t {ChannelA = 1, ChannelB = 2, ChannelC = 4, ChannelD = 8};

// Handler for MAX5134 4 channel 16 bit DAC
class DACHandler
{
public:
	void Init();
	void sendData(uint8_t cmd, uint16_t data);
};

extern DACHandler dacHandler;
