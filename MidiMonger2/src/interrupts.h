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


