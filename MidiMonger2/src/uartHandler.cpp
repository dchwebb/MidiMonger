#include "uartHandler.h"

volatile uint8_t uartCmdPos = 0;
volatile char uartCmd[100];
volatile bool uartCmdRdy = false;

// Manages communication to ST Link debugger UART

void InitUART() {
	// 446 Nucleo uses PD8 (TX) PD9 (RX) for USART2

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;			// UART clock enable

	GpioPin::Init(GPIOA, 2, GpioPin::Type::AlternateFunction, 7);		// TX
	GpioPin::Init(GPIOA, 3, GpioPin::Type::AlternateFunction, 7);		// RX

	int Baud = (SystemCoreClock / 4) / (16 * 230400);		// NB must be an integer or timing will be out
	//int Baud = (SystemCoreClock / 4) / (16 * 31250);
	USART2->BRR |= Baud << 4;						// Baud Rate (called USART_BRR_DIV_Mantissa) = (Sys Clock: 180MHz / APB1 Prescaler DIV4: 45MHz) / (16 * 31250) = 90
	USART2->BRR |= 12;								// Fraction: (144MHz / 4) / (16 * 230400) = 9.765625: multiply remainder by 16: 16 * .765625 = 12.25
	USART2->CR1 &= ~USART_CR1_M;					// Clear bit to set 8 bit word length
	USART2->CR1 |= USART_CR1_RE;					// Receive enable
	USART2->CR1 |= USART_CR1_TE;					// Transmitter enable

	// Set up interrupts
	USART2->CR1 |= USART_CR1_RXNEIE;
	NVIC_SetPriority(USART2_IRQn, 3);				// Lower is higher priority
	NVIC_EnableIRQ(USART2_IRQn);

	USART2->CR1 |= USART_CR1_UE;					// USART Enable

}

std::string IntToString(const int32_t& v) {
	std::stringstream ss;
	ss << v;
	return ss.str();
}

std::string HexToString(const uint32_t& v, const bool& spaces) {
	std::stringstream ss;
	ss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << v;
	if (spaces) {
		//std::string s = ss.str();
		return ss.str().insert(2, " ").insert(5, " ").insert(8, " ");
	}
	return ss.str();
}

std::string HexByte(const uint16_t& v) {
	std::stringstream ss;
	ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << v;
	return ss.str();
}



size_t uartSendStr(const unsigned char* s, size_t len)
{
	for (uint32_t i = 0; i < len; ++i) {
		while ((USART2->SR & USART_SR_TXE) == 0);
		USART2->DR = s[i];
	}
	return len;
}

void uartSendStr(const std::string& s) {
	for (char c : s) {
		while ((USART2->SR & USART_SR_TXE) == 0);
		USART2->DR = c;
	}
}

extern "C" {

void USART2_IRQHandler(void) {
	if (USART2->SR | USART_SR_RXNE && !uartCmdRdy) {
		uartCmd[uartCmdPos] = USART2->DR; 				// accessing DR automatically resets the receive flag
		if (uartCmd[uartCmdPos] == 10) {
			uartCmdRdy = true;
			uartCmdPos = 0;
		} else {
			uartCmdPos++;
		}
	}
}


}
