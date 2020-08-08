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

// test routine that plays a short note when Virtial COM port receives text 'test'
void CDCHandler(uint8_t* data, uint32_t length) {
	std::string s = std::string((char*)data, length);
	if (s.compare("test")) {
		MidiData midiEvent;
		midiEvent.chn = 0;
		midiEvent.msg = NoteOn;
		midiEvent.db1 = 50;
		midiHandler.midiEvent(midiEvent.data);

		for (int i = 0; i < 1000000; ++i);
		midiEvent.msg = NoteOff;
		midiHandler.midiEvent(midiEvent.data);
	}
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
	usb.midiDataHandler = std::bind(&MidiHandler::eventHandler, &midiHandler, std::placeholders::_1, std::placeholders::_2);
	usb.cdcDataHandler = std::bind(CDCHandler, std::placeholders::_1, std::placeholders::_2);

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

