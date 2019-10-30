#pragma once

#include "initialisation.h"

union DacData {
	struct {
		uint8_t addr: 3, cmd: 5;
		uint16_t data;
		uint8_t blank;
	};
	uint8_t da[3];
};

class DACHandler
{
public:
	//void sendData(uint32_t data);
	void sendData(DacData data);
};
