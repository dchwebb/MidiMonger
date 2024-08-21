void SysTick_Handler(void)
{
	++SysTickVal;
}

void OTG_FS_IRQHandler(void)
{
	if (hostMode) {
		usbHost.IRQHandler();
	} else {
		usb.InterruptHandler();
	}
}

// MIDI Decoder
void UART4_IRQHandler(void) {
	if (UART4->SR | USART_SR_RXNE) {
		midiControl.SerialHandler(UART4->DR); 				// accessing DR automatically resets the receive flag
	}
}

void NMI_Handler(void)
{
	while (1) {
	}
}

void HardFault_Handler(void)
{
	while (1) {
	}
}

void MemManage_Handler(void)
{
	while (1) {
	}
}

void BusFault_Handler(void)
{
	while (1) {
	}
}

void UsageFault_Handler(void)
{
	while (1) {
	}
}


void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}


