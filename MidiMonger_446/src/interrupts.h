void OTG_FS_IRQHandler(void) {
	usb.USBInterruptHandler();
}

void SysTick_Handler(void) {
	SysTickVal++;
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

