#include <config.h>

// called whenever a config setting is changed to schedule a save after waiting to see if any more changes are being made
void Config::ScheduleSave() {
	scheduleSave = true;
	saveBooked = SysTickVal;
}

// Write calibration settings to Flash memory
void Config::SaveConfig() {
	// Check if save is scheduled and ready
	if (!scheduleSave || saveBooked + 10000 < SysTickVal)
		return;

	scheduleSave = false;

	uint32_t address = ADDR_FLASH_SECTOR_7;		// Store data in Sector 7 last sector in F446 to allow maximum space for program code
	FLASH_Status flash_status = FLASH_COMPLETE;

	configValues cv;
	SetConfig(cv);

	__disable_irq();		// Disable Interrupts
	FLASH_Unlock();			// Unlock Flash memory for writing

	// Clear error flags in Status Register
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	// Erase sector 7 (has to be erased before write - this sets all bits to 1 as write can only switch to 0)
	flash_status = FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3);

	// If erase worked, program the Flash memory with the config settings byte by byte
	if (flash_status == FLASH_COMPLETE) {
		for (unsigned int f = 0; f < sizeof(cv); f++) {
			char byte = *((char*)(&cv) + f);
			flash_status = FLASH_ProgramByte((uint32_t)address + f, byte);
		}
	}

	FLASH_Lock();			// Lock the Flash memory
	__enable_irq(); 		// Enable Interrupts
}

void Config::SetConfig(configValues &cv) {
	cv.pitchBendSemiTones = midiHandler.pitchBendSemiTones;
	for (uint8_t g = 0; g < 8; g++) {
		cv.gate[g].type = (uint8_t)midiHandler.gateOutputs[g].type;
		cv.gate[g].channel = midiHandler.gateOutputs[g].channel;
		cv.gate[g].note = midiHandler.gateOutputs[g].note;
	}
	for (uint8_t c = 0; c < 4; c++) {
		cv.cv[c].type = (uint8_t)midiHandler.cvOutputs[c].type;
		cv.cv[c].channel = midiHandler.cvOutputs[c].channel;
		cv.cv[c].controller = midiHandler.cvOutputs[c].controller;
	}
}


// Restore configuration settings from flash memory
void Config::RestoreConfig()
{
	// create temporary copy of settings from memory to check if they are valid
	configValues cv;
	memcpy(&cv, (uint32_t*)ADDR_FLASH_SECTOR_7, sizeof(cv));

	if (strcmp(cv.StartMarker, "CFG") == 0 && strcmp(cv.EndMarker, "END") == 0 && cv.Version == 1) {
		midiHandler.pitchBendSemiTones = cv.pitchBendSemiTones;
		for (uint8_t g = 0; g < 8; g++) {
			midiHandler.gateOutputs[g].type = (gateType)cv.gate[g].type;
			midiHandler.gateOutputs[g].channel = cv.gate[g].channel;
			midiHandler.gateOutputs[g].note = cv.gate[g].note;
		}
		for (uint8_t c = 0; c < 4; c++) {
			midiHandler.cvOutputs[c].type = (cvType)cv.cv[c].type;
			midiHandler.cvOutputs[c].channel = cv.cv[c].channel;
			midiHandler.cvOutputs[c].controller = cv.cv[c].controller;
		}
	}
}


