#include "initialisation.h"
#include "USBHost.h"
#include "USB.h"
#include "HidHostClass.h"
#include "CommandHandler.h"
#include "uartHandler.h"
#include "configManager.h"
#include "MidiControl.h"
#include <stdio.h>

volatile uint32_t SysTickVal;
bool hostMode = true;

extern "C" {
#include "interrupts.h"
}

Config config{&midiControl.configSaver, &hidHostClass.configSaver};		// Construct config handler with list of configSavers


int main(void)
{
	InitHardware();
	uart.Init();
	dacHandler.Init();
	config.RestoreConfig();

	hostMode = modeSwitch.IsLow();
	if (hostMode) {
		usbHost.Init();
	} else {
		usb.Init(false);
	}

	printf("\r\n\r\nStarting ...\r\n");
	midiControl.LightShow(MidiControl::LightShowType::startup);

	while (1) {
		midiControl.GateTimer();			// Switches off any pending gates/leds
		config.SaveConfig();				// Save any scheduled changes
		commandHandler.CheckCommands();		// Check if any commands received via USB serial or UART

		if (hostMode) {
			usbHost.Process();
		}

		if (modeSwitch.IsLow() != hostMode) {			// Switch between USB host and device mode
			DelayMS(2);
			if (modeSwitch.IsHigh()) {
				printf("Switching to device mode\r\n");
				usbHost.Disable();
				DelayMS(10);
				hostMode = false;
				usb.Init(false);
			} else {
				printf("Switching to host mode\r\n");
				usb.Disable();
				DelayMS(10);
				hostMode = true;
				usbHost.Init();
			}
		}
	}
}


