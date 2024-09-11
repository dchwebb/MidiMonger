#include "uartHandler.h"
#include "CommandHandler.h"

UARTHandler uart;


void UARTHandler::Init()
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;			// UART clock enable

	GpioPin::Init(GPIOA, 2, GpioPin::Type::AlternateFunction, 7);		// TX
#ifndef V1_HARDWARE
	GpioPin::Init(GPIOA, 3, GpioPin::Type::AlternateFunction, 7);		// RX
#endif

	const float baud = (float)(SystemCoreClock / 4) / (16.0f * 230400.0f);		// NB must be an integer or timing will be out
	const uint32_t mantissa = (uint32_t)baud;									// Baud rate integer part
	const uint32_t fraction = (uint32_t)((baud - mantissa) * 16.0f);			// Fraction: baud rate fractional part * 16

	USART2->BRR |= (mantissa << USART_BRR_DIV_Mantissa_Pos) |
				   (fraction << USART_BRR_DIV_Fraction_Pos);
	USART2->CR1 &= ~USART_CR1_M;					// Clear bit to set 8 bit word length
	USART2->CR1 |= USART_CR1_RE;					// Receive enable
	USART2->CR1 |= USART_CR1_TE;					// Transmitter enable

	// Set up interrupts
	USART2->CR1 |= USART_CR1_RXNEIE;
	NVIC_SetPriority(USART2_IRQn, 3);				// Lower is higher priority
	NVIC_EnableIRQ(USART2_IRQn);

	USART2->CR1 |= USART_CR1_UE;					// USART Enable

}


void UARTHandler::ProcessCommand()
{
	if (!cmdPending) {
		return;
	}

	std::string_view cmd {command};
	commandHandler.ProcessCommand(cmd);
	cmdPending = false;
}


size_t UARTHandler::SendString(const unsigned char* s, size_t len)
{
	for (uint32_t i = 0; i < len; ++i) {
		while ((USART2->SR & USART_SR_TXE) == 0);
		USART2->DR = s[i];
	}
	return len;
}


void UARTHandler::SendString(const std::string& s)
{
	for (char c : s) {
		while ((USART2->SR & USART_SR_TXE) == 0);
		USART2->DR = c;
	}
}


void UARTHandler::DataIn(char data)
{
	if (!cmdPending) {
		command[cmdPos] = data;
		if (command[cmdPos] == 13 || command[cmdPos] == 10 || cmdPos == maxCmdLen - 1) {
			command[cmdPos] = 0;
			cmdPending = true;
			cmdPos = 0;
		} else {
			++cmdPos;
		}
	}
}
