#include <DACHandler.h>

/*void DACHandler::sendData(uint32_t data) {
	uint8_t* sendData = (uint8_t*)&data;
	SPI3->CR1 |= SPI_CR1_SPE;						// Enable SPI
	for (int8_t i = 2; i >= 0; --i) {				// DAC requires 24 bits: send as three 8-bit packets
		SPI3->DR = *(sendData + i);

		while(((SPI3->SR & SPI_SR_TXE) == 0) | ((SPI3->SR & SPI_SR_BSY) == SPI_SR_BSY) );
	}
	SPI3->CR1 &= ~SPI_CR1_SPE;						// Disable SPI

}*/

void DACHandler::sendData(DacData data) {
	uint8_t* sendData = (uint8_t*)&data;
	SPI3->CR1 |= SPI_CR1_SPE;						// Enable SPI
	for (uint8_t i = 0; i < 3; ++i) {				// DAC requires 24 bits: send as three 8-bit packets
		SPI3->DR = sendData[i];

		while(((SPI3->SR & SPI_SR_TXE) == 0) | ((SPI3->SR & SPI_SR_BSY) == SPI_SR_BSY) );
	}
	SPI3->CR1 &= ~SPI_CR1_SPE;						// Disable SPI

}


/*

void DACHandler::sendData(DacData data) {
	uint8_t* sendData = (uint8_t*)&data;
	GPIOA->BSRR |= GPIO_BSRR_BR_15;
	for (uint8_t i = 0; i < 3; ++i) {				// DAC requires 24 bits: send as three 8-bit packets
		SPI3->DR = sendData[i];

		while(((SPI3->SR & SPI_SR_TXE) == 0) | ((SPI3->SR & SPI_SR_BSY) == SPI_SR_BSY) );
	}
	GPIOA->BSRR |= GPIO_BSRR_BS_15;

}
*/
