#pragma once

#include "initialisation.h"

// Handler for MAX5134 4 channel 16 bit DAC
class DACHandler
{
public:
	enum Command : uint8_t {WriteChannel = 0b0011 << 4, Clear = 0b0010, Linearity = 0b101};
	enum Address : uint8_t {ChannelA = 1, ChannelB = 2, ChannelC = 4, ChannelD = 8};

	void Init();
	void SendData(uint8_t cmd, uint16_t data);
private:
	GpioPin mosi {GPIOB, 5, GpioPin::Type::AlternateFunction, 6, GpioPin::DriveStrength::High};		// PB5 SPI_MOSI AF6
	GpioPin clk {GPIOB, 3, GpioPin::Type::AlternateFunction, 6, GpioPin::DriveStrength::High};		// PB3 SPI_SCK AF6
	GpioPin nss {GPIOA, 15, GpioPin::Type::Output};													// PA15 SPI_NSS
};

extern DACHandler dacHandler;
