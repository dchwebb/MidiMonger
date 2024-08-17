#include <DACHandler.h>

DACHandler dacHandler;

void DACHandler::SendData(uint8_t cmd, uint16_t data) {

	nss.SetLow();
	SPI3->DR = cmd;		// Send cmd data [X X C C C A A A]
	while ((SPI3->SR & SPI_SR_TXE) == 0);

	SPI3->DR = (uint8_t)(data >> 8);				// Send data high byte
	while ((SPI3->SR & SPI_SR_TXE) == 0);

	SPI3->DR = (uint8_t)(data & 0xFF);				// Send data low byte
	while ((SPI3->SR & SPI_SR_TXE) == 0);
	while (((SPI3->SR & SPI_SR_TXE) == 0) | ((SPI3->SR & SPI_SR_BSY) == SPI_SR_BSY) );
	nss.SetHigh();
}


void DACHandler::Init()
{
	RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

	SPI3->CR1 |= SPI_CR1_BR_0;						// Baud rate control prescaler: 0b001: fPCLK/4; 0b100: fPCLK/32
	SPI3->CR1 |= SPI_CR1_MSTR;						// Master selection
	SPI3->CR1 |= SPI_CR1_SSM;						// Software slave management: When SSM bit is set, NSS pin input is replaced with the value from the SSI bit
	SPI3->CR1 |= SPI_CR1_SSI;						// Internal slave select
	SPI3->CR1 |= SPI_CR1_SPE;						// Enable SPI

	nss.SetHigh();

	SendData(Clear, 0);		// Clear/reset DAC

	// calibrate linearity: To guarantee DAC linearity, wait until the supplies have settled. Set the LIN bit in the DAC linearity register; wait 10ms, and clear the LIN bit.
	SendData(Linearity, (1 << 9));
	DelayMS(10);
	SendData(Linearity, 0);
}
