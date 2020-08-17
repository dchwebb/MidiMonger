#include "uartHandler.h"

// Manages communication to ST Link debugger UART

void InitUART() {
	// 446 Nucleo uses PD8 (TX) PD9 (RX) for USART3

	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;			// UART clock enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;			// GPIO port enable

	GPIOD->MODER |= GPIO_MODER_MODER8_1;			// Set alternate function on PA9
	GPIOD->AFR[1] |= 7 << GPIO_AFRH_AFSEL8_Pos;		// Alternate function on PD8 for UART3_TX is AF7
	GPIOD->MODER |= GPIO_MODER_MODER9_1;			// Set alternate function on PA10
	GPIOD->AFR[1] |= 7 << GPIO_AFRH_AFSEL9_Pos;		// Alternate function on PD9 for UART3_RX is AF7

	int Baud = (SystemCoreClock / 4) / (16 * 230400);		// NB must be an integer or timing will be out
	//int Baud = (SystemCoreClock / 4) / (16 * 31250);
	USART3->BRR |= Baud << 4;						// Baud Rate (called USART_BRR_DIV_Mantissa) = (Sys Clock: 180MHz / APB1 Prescaler DIV4: 45MHz) / (16 * 31250) = 90
	USART3->BRR |= 12;								// Fraction: (144MHz / 4) / (16 * 230400) = 9.765625: multiply remainder by 16: 16 * .765625 = 12.25
	USART3->CR1 &= ~USART_CR1_M;					// Clear bit to set 8 bit word length
	USART3->CR1 |= USART_CR1_RE;					// Receive enable
	USART3->CR1 |= USART_CR1_TE;					// Transmitter enable

	// Set up interrupts
	USART3->CR1 |= USART_CR1_RXNEIE;
	NVIC_SetPriority(USART3_IRQn, 3);				// Lower is higher priority
	NVIC_EnableIRQ(USART3_IRQn);

	USART3->CR1 |= USART_CR1_UE;					// USART Enable

	// configure GPIO to act as button on nucleo board (as user button is already a cv output)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR11_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	GPIOB->MODER &= ~GPIO_MODER_MODE11_Msk;

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

void uartSendStr(const std::string& s) {
	for (char c : s) {
		while ((USART3->SR & USART_SR_TXE) == 0);
		USART3->DR = c;
	}
}
