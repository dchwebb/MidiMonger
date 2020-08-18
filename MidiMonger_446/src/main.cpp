#include "initialisation.h"
#include "USB.h"
#include "MidiHandler.h"
#include "DACHandler.h"
#include "Config.h"
#include "CDCHandler.h"

volatile uint32_t SysTickVal;

bool CmdPending = false;
std::string ComCmd;

USB usb;
MidiHandler midiHandler;
DACHandler dacHandler;
Config cfg;

uint32_t debugClock = 0;
uint32_t debugClDiff = 0;
bool USBDebug;


extern "C" {
#include "interrupts.h"
}

// As this is called from an interrupt assign the command to a variable so it can be handled in the main loop
void CDCHandler(uint8_t* data, uint32_t length) {
	ComCmd = std::string((char*)data, length);
	CmdPending = true;
}


void LilLightShow() {
	for (uint8_t c = 0; c < 8; ++c) {
		midiHandler.cvOutputs[c > 3 ? 3 - c % 4 : c].ledOn(60);
		uint32_t delay = SysTickVal + 110;
		while (delay > SysTickVal) {
			midiHandler.gateTimer();
		}
	}
}

extern uint32_t SystemCoreClock;
int main(void)
{
	SystemInit();							// Activates floating point coprocessor and resets clock
	SystemClock_Config();					// Configure the clock and PLL
	SystemCoreClockUpdate();				// Update SystemCoreClock (system clock frequency) derived from settings of oscillators, prescalers and PLL
	InitSysTick();
	dacHandler.initDAC();
	InitMidiUART();
	usb.InitUSB();
	cfg.RestoreConfig();
	midiHandler.setConfig();

	// Bind the usb.dataHandler function to the midiHandler's event handler
	usb.midiDataHandler = std::bind(&MidiHandler::eventHandler, &midiHandler, std::placeholders::_1, std::placeholders::_2);
	usb.cdcDataHandler = std::bind(CDCHandler, std::placeholders::_1, std::placeholders::_2);

	// Little light show
	LilLightShow();



	while (1)
	{

		midiHandler.gateTimer();		// Switches off any pending gates

		// Check for incoming CDC commands
		if (CmdPending) {
			if (!CDCCommand(ComCmd)) {
				usb.SendString("Unrecognised command. Type 'help' for supported commands\n");
			}
			CmdPending = false;
		}

		cfg.SaveConfig();		// Checks if configuration change is pending a save

		// Debug mode - output USB trace via System UART
#if (USB_DEBUG)
		if ((GPIOB->IDR & GPIO_IDR_ID11) == 0 && !USBDebug) {
			USBDebug = true;
			usb.OutputDebug();
		} else {
			USBDebug = false;
		}
#endif
	}
}

