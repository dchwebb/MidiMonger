#pragma once

#include "initialisation.h"
#include "MidiHandler.h"
#include <cstring>

#include "../drivers/stm32f4xx_flash.h"

#define ADDR_FLASH_SECTOR_7		((uint32_t)0x08060000) // Base address of Sector 7, 128 Kbytes

class MidiHandler;		// forward reference to handle circular dependency
extern MidiHandler midiHandler;


struct cfgGate {
	uint8_t type;
	uint8_t channel;
	uint8_t note;
};

struct cfgCV {
	uint8_t type;
	uint8_t channel;
	uint8_t controller;
};


struct configValues {
	char StartMarker[4] = "CFG";		// Start Marker

	//	General settings
	uint8_t Version = 1;				// version of saved config struct format
	uint8_t pitchBendSemiTones = 0;

	cfgGate gate[8];
	cfgCV cv[4];

	char EndMarker[4] = "END";			// End Marker
};


// Class used to store calibration settings - note this uses the Standard Peripheral Driver code
class Config {
public:
	bool scheduleSave = false;
	uint32_t saveBooked;

	void ScheduleSave();				// called whenever a config setting is changed to schedule a save after waiting to see if any more changes are being made
	void SaveConfig();
	void SetConfig(configValues &cv);	// sets properties of class to match current values
	void RestoreConfig();				// gets config from Flash, checks and updates settings accordingly

};

