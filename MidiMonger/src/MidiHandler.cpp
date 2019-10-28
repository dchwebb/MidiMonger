#include <MidiHandler.h>

void MidiHandler::eventHandler(MidiData midiEvent)
{
	// Note On
	if (midiEvent.msg == 9) {
		// Delete note if already playing and add to latest position in list
		midiNotes.remove(midiEvent.db1);
		midiNotes.push_back(midiEvent.db1);
	}

	// Note Off
	if (midiEvent.msg == 8) {
		midiNotes.remove(midiEvent.db1);
	}

	// Request for information from editor
	if (midiEvent.msg == 0xF && midiEvent.chn == 2) {
		MidiData tx = {0x2C2DF203};
		usb.SendReport((uint8_t*) &tx, 4);
	}

	// Set pitch
	if (midiNotes.size() > 0) {
		uint16_t dacOut = 4095 * (float)(std::min(std::max((int)midiNotes.back(), 24), 96) - 24) / 72;		// limit C1 to C7
		DAC->DHR12R1 = dacOut;
	}

	// light up LED (PB14) and transmit gate (PA3)
	if (midiNotes.size() > 0) {
		GPIOB->BSRR |= GPIO_BSRR_BS_14;
		GPIOA->BSRR |= GPIO_BSRR_BS_3;
	}
	else {
		GPIOB->BSRR |= GPIO_BSRR_BR_14;
		GPIOA->BSRR |= GPIO_BSRR_BR_3;
	}

}

void MidiHandler::getMidiData(uint32_t data) {
	eventHandler((MidiData)data);
}
