#include "initialisation.h"
#include "USBHost.h"
#include "USB.h"
#include "uartHandler.h"
#include "configManager.h"
#include "MidiControl.h"
#include <stdio.h>

volatile uint32_t SysTickVal;
bool hostMode = true;

extern "C" {
#include "interrupts.h"

size_t _write(int handle, const unsigned char* buf, size_t len)
{
	if (hostMode) {
		return uartSendStr(buf, len);							// Logging via UART
	} else {
		if (usb.devState == USB::DeviceState::Configured) {		// Logging via USB
			return usb.SendString(buf, len);
		} else {
			return 0;
		}
	}

}
}

Config config{&midiControl.configSaver};	// Construct config handler with list of configSavers


int main(void)
{
	InitHardware();
	InitUART();
	dacHandler.Init();
	config.RestoreConfig();

	hostMode = modeSwitch.IsHigh();
	if (hostMode) {
		usbHost.Init();
	} else {
		usb.Init(false);
	}

	printf("\r\n\r\nStarting ...\r\n");
	midiControl.LightShow();

	while (1) {
		midiControl.GateTimer();			// Switches off any pending gates/leds
		config.SaveConfig();				// Save any scheduled changes

		if (hostMode) {
			usbHost.Process();
		} else {
			usb.cdc.ProcessCommand();		// Check for incoming USB serial commands
		}

		if (modeSwitch.IsHigh() != hostMode) {			// Switch between USB host and device mode
			DelayMS(2);
			if (modeSwitch.IsHigh()) {
				printf("Switching to device mode\r\n");
				usbHost.Disable();
			} else {
				printf("Switching to host mode\r\n");
				usbHost.Init();
			}
			hostMode = !hostMode;
		}
	}
}


