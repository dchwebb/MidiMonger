#include "initialisation.h"
#include "USB.h"
#include "MidiHandler.h"
#include "DACHandler.h"
#include "Config.h"
#include <sstream>

volatile uint32_t SysTickVal;

//bool noteDown = false;

bool CmdPending = false;
std::string ComCmd;

USB usb;
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
	ComCmd = std::string((char*)data, length);
	CmdPending = true;
}

std::string IntToString(const int32_t& v) {
	std::stringstream ss;
	ss << v;
	return ss.str();
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
	InitUART();
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
			if (ComCmd.compare("help\n") == 0) {
				usb.SendString("Mountjoy MIDI Monger - supported commands:\n\n");
				usb.SendString("help  -  Shows this information\n");
				usb.SendString("config  -  Shows current control configuration\n");
				usb.SendString("test  -  Light display\n");
				usb.SendString("nVcC  -  Play note vlue V on channel C. Eg \'n60c1\' is middle C on channel 1\n");

			} else if (ComCmd.compare("config\n") == 0) {

				std::stringstream ss;
				ss << "Configuration:\n";
				uint8_t gate = 0;
				for (Gate& g : midiHandler.gateOutputs) {
					ss << "\nGate " << ++gate << ": ";
					switch(g.type) {
					case gateType::channelNote:
						ss << "Channel Gate. Channel: " << (int)g.channel;
						break;
					case gateType::specificNote:
						ss << "Specific Note. Channel: " << IntToString(g.channel) << " Note: " << (int)g.note;
						break;
					case gateType::clock: ss << "Clock"; break;
					}
				}

				ss << '\n';
				uint8_t c = 0;
				for (MidiHandler::CV& cv : midiHandler.cvOutputs) {
					ss << "\nCV " << ++c << ": ";
					switch (cv.type) {
					case cvType::channelPitch:
						ss << "Channel Pitch";
						break;
					case cvType::controller:
						ss << "Controller: " << (int)cv.controller;
						break;
					case cvType::pitchBend:
						ss << "Pitchbend";
						break;
					case cvType::afterTouch:
						ss << "Aftertouch";
						break;
					}
					ss << ". Channel: " << (int)cv.channel;
				}
				usb.SendString(ss.str().c_str());

			} else if (ComCmd.compare("test\n") == 0) {
				LilLightShow();
			} else 	if (ComCmd.compare(0, 1, "n") == 0) {

				int8_t cpos = ComCmd.find("c");		// locate position of channel code

				// Check that command is correctly formed - note that stoi can throw exceptions which breaks program
				if (cpos > 1 && std::strspn(ComCmd.substr(1).c_str(), "0123456789") > 0 && std::strspn(ComCmd.substr(cpos + 1).c_str(), "0123456789") > 0) {
					uint8_t noteVal = stoi(ComCmd.substr(1, cpos));
					std::string s = ComCmd.substr(cpos + 1);
					uint8_t channel = stoi(ComCmd.substr(cpos + 1));

					MidiData midiEvent;
					midiEvent.chn = channel - 1;
					midiEvent.msg = NoteOn;
					midiEvent.db1 = noteVal;
					midiHandler.midiEvent(midiEvent.data);

					uint32_t delay = SysTickVal + 1000;
					while (delay > SysTickVal);

					midiEvent.msg = NoteOff;
					midiHandler.midiEvent(midiEvent.data);
				} else {
					usb.SendString("Note command incorrectly formed\n");
				}
			}
			CmdPending = false;
		}


		cfg.SaveConfig();		// Checks if configuration change is pending a save
	}
}

