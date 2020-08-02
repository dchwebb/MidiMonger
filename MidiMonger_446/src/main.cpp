#include "initialisation.h"
#include "USB.h"
#include "MidiHandler.h"
#include "DACHandler.h"
#include "Config.h"

USB usb;
uint32_t usbEvents[200];
uint8_t usbEventNo = 0;
uint8_t reqEventNo = 0;
uint8_t midiEventRead = 0;
uint8_t midiEventWrite = 0;
uint8_t eventOcc = 0;
uint16_t noteOnTest = 0;
volatile uint32_t SysTickVal;

bool noteDown = false;

MidiData midiArray[MIDIBUFFERSIZE];		// for debugging
MidiHandler midiHandler;
DACHandler dacHandler;
Config cfg;

uint32_t debugClock = 0;
uint32_t debugClDiff = 0;

extern "C" {
#include "interrupts.h"
}

extern uint32_t SystemCoreClock;
int main(void)
{
	SystemInit();							// Activates floating point coprocessor and resets clock
	SystemClock_Config();					// Configure the clock and PLL
	SystemCoreClockUpdate();				// Update SystemCoreClock (system clock frequency) derived from settings of oscillators, prescalers and PLL
	usb.InitUSB();
	InitSysTick();
	dacHandler.initDAC();
	InitUART();
	cfg.RestoreConfig();
	midiHandler.setConfig();

	// Bind the usb.dataHandler function to the midiHandler's event handler
	usb.dataHandler = std::bind(&MidiHandler::eventHandler, &midiHandler, std::placeholders::_1, std::placeholders::_2);

	// Little light show
	for (uint8_t c = 0; c < 8; ++c) {
		midiHandler.cvOutputs[c > 3 ? 3 - c % 4 : c].ledOn(60);
		uint32_t delay = SysTickVal + 110;
		while (delay > SysTickVal) {
			midiHandler.gateTimer();
		}

	}

	while (1)
	{
		midiHandler.gateTimer();
		cfg.SaveConfig();		// Checks if configuration change is pending a save
	}
}

