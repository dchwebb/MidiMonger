void OTG_FS_IRQHandler(void) {
	usb.USBInterruptHandler();
}

void SysTick_Handler(void) {
	SysTickVal++;
}

// Dev board ST Link UART Handler
void USART3_IRQHandler(void) {
	if (USART3->SR | USART_SR_RXNE && !uartCmdRdy) {
		uartCmd[uartCmdPos] = USART3->DR; 				// accessing DR automatically resets the receive flag
		if (uartCmd[uartCmdPos] == 10) {
			uartCmdRdy = true;
			uartCmdPos = 0;
		} else {
			uartCmdPos++;
		}
	}
}

// MIDI Decoder
void UART4_IRQHandler(void) {
	if (UART4->SR | USART_SR_RXNE) {
		midiHandler.serialHandler(UART4->DR); 				// accessing DR automatically resets the receive flag
	}
}

void NMI_Handler(void) {}

void HardFault_Handler(void) {
	while (1) {}
}

void MemManage_Handler(void) {
	while (1) {}
}

void BusFault_Handler(void) {
	while (1) {}
}

void UsageFault_Handler(void) {
	while (1) {}
}

void SVC_Handler(void) {}

void DebugMon_Handler(void) {}

void PendSV_Handler(void) {}

