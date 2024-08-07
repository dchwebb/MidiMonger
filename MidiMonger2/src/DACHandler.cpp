#include <DACHandler.h>

DACHandler dacHandler;

void DACHandler::sendData(uint8_t cmd, uint16_t data) {

	GPIOA->BSRR |= GPIO_BSRR_BR_15;
	SPI3->DR = cmd;		// Send cmd data [X X C C C A A A]
	while ((SPI3->SR & SPI_SR_TXE) == 0);

	SPI3->DR = (uint8_t)(data >> 8);				// Send data high byte
	while ((SPI3->SR & SPI_SR_TXE) == 0);

	SPI3->DR = (uint8_t)(data & 0xFF);				// Send data low byte
	while ((SPI3->SR & SPI_SR_TXE) == 0);
	while (((SPI3->SR & SPI_SR_TXE) == 0) | ((SPI3->SR & SPI_SR_BSY) == SPI_SR_BSY) );
	GPIOA->BSRR |= GPIO_BSRR_BS_15;
}


void DACHandler::Init()
{
	//	Enable GPIO and SPI clocks
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// reset and clock control - advanced high performance bus - GPIO port A
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;			// reset and clock control - advanced high performance bus - GPIO port B
	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

	// PB5: SPI_MOSI [alternate function AF6]
	GPIOB->MODER |= GPIO_MODER_MODER5_1;			// 00: Input (reset state)	01: General purpose output mode	10: Alternate function mode	11: Analog mode
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;		// V High  - 00: Low speed; 01: Medium speed; 10: High speed; 11: Very high speed
	GPIOB->AFR[0] |= 0b0110 << 20;					// 0b0110 = Alternate Function 6 (SPI3); 20 is position of Pin 5

	// PB3 SPI_SCK [alternate function AF6]
	GPIOB->MODER &= ~GPIO_MODER_MODER3;				// Reset value of PB3 is 0b10
	GPIOB->MODER |= GPIO_MODER_MODER3_1;			// 00: Input (reset state)	01: General purpose output mode	10: Alternate function mode	11: Analog mode
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;		// V High  - 00: Low speed; 01: Medium speed; 10: High speed; 11: Very high speed
	GPIOB->AFR[0] |= 0b0110 << 12;					// 0b0110 = Alternate Function 6 (SPI3); 12 is position of Pin 3

	SPI3->CR1 |= SPI_CR1_BR_0;						// Baud rate control prescaler: 0b001: fPCLK/4; 0b100: fPCLK/32
	SPI3->CR1 |= SPI_CR1_MSTR;						// Master selection

	// see page 853 for details of configuring NSS
	GPIOA->MODER &= ~GPIO_MODER_MODER15;
	GPIOA->MODER |= GPIO_MODER_MODER15_0;			// 00: Input (reset state)	01: General purpose output mode	10: Alternate function mode	11: Analog mode
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR15;

	SPI3->CR1 |= SPI_CR1_SSM;						// Software slave management: When SSM bit is set, NSS pin input is replaced with the value from the SSI bit
	SPI3->CR1 |= SPI_CR1_SSI;						// Internal slave select
	SPI3->CR1 |= SPI_CR1_SPE;						// Enable SPI

	GPIOA->BSRR |= GPIO_BSRR_BS_15;

	sendData(Clear, 0);		// Clear/reset DAC

	// calibrate linearity: To guarantee DAC linearity, wait until the supplies have settled. Set the LIN bit in the DAC linearity register; wait 10ms, and clear the LIN bit.
	sendData(Linearity, (1 << 9));
	uint32_t start = SysTickVal;		// each tick is around 400us: 25 x 400us = 10ms
	while (SysTickVal < start + 25);
	sendData(Linearity, 0);
}
