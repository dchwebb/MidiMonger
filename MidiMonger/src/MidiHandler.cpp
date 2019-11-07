#include <MidiHandler.h>

extern MidiData midiArray[MIDIBUFFERSIZE];

MidiHandler::MidiHandler() {

}
void MidiHandler::eventHandler(uint32_t data)
{
	/*
	Message Type	MS Nybble	LS Nybble		Bytes		Data Byte 1			Data Byte 2
	-----------------------------------------------------------------------------------------
	Note Off		0x8			Channel			2			Note Number			Velocity
	Note On			0x9			Channel			2			Note Number			Velocity
	Poly Pressure	0xA			Channel			2			Note Number			Pressure
	Control Change	0xB			Channel			2			Controller 			Value
	Program Change	0xC			Channel			1			Program Number		-none-
	Ch. Pressure	0xD			Channel			1			Pressure			-none-
	Pitch Bend		0xE			Channel			2			Bend LSB (7-bit)	Bend MSB (7-bits)
	System			0xF			further spec	variable	variable			variable
	*/

	MidiData midiEvent = MidiData(data);

	// Store Midi events to array for debugging
	midiArray[midiEventRead] = midiEvent;
	midiEventRead = midiEventRead == MIDIBUFFERSIZE ? 0 : midiEventRead + 1;

	// Editor communication
	if (midiEvent.db0 == 0xF2) {
		// Request for information from editor. Format is: Code = 0x03; db0 = 0xF2; db1 = 0xnn (output number)
		if (midiEvent.db0 == 0xF2 && midiEvent.configType == 0) {
			MidiData tx;
			tx.Code = 0x03;
			tx.db0 = 0xF2;
			if (midiEvent.db1 < 9) {
				// Gate outputs (1 - 8)
				tx.configType = (uint8_t)gateOutputs[midiEvent.db1 - 1].type;
				tx.cfgChannelOrOutput = gateOutputs[midiEvent.db1 - 1].channel;
				tx.configValue = gateOutputs[midiEvent.db1 - 1].note;
			} else {
				// CV outputs (9 - 12)
				tx.configType = (uint8_t)cvOutputs[midiEvent.db1 - 9].type;
				tx.cfgChannelOrOutput = cvOutputs[midiEvent.db1 - 9].channel;
				tx.configValue = cvOutputs[midiEvent.db1 - 9].controller;
			}

			usb.SendReport((uint8_t*) &tx, 4);
		} else {
			// configuration changed by editor format is ttttoooo vvvvvvvv where t is type of setting, o is output number (1-8 = gate, 9 - 12 = cv) and v is setting value
			if (midiEvent.cfgChannelOrOutput < 9) {
				// Gate configuration
				switch ((configSetting)midiEvent.configType) {
				case configSetting::type :
					gateOutputs[midiEvent.cfgChannelOrOutput - 1].type = (gateType)midiEvent.configValue;
					break;
				case configSetting::specificNote :
					gateOutputs[midiEvent.cfgChannelOrOutput - 1].note = midiEvent.configValue;
					break;
				case configSetting::channel :
					gateOutputs[midiEvent.cfgChannelOrOutput - 1].channel = midiEvent.configValue;
					break;
				}
			} else {
				// CV Configuration
				switch ((configSetting)midiEvent.configType) {
				case configSetting::type :
					cvOutputs[midiEvent.cfgChannelOrOutput - 9].type = (cvType)midiEvent.configValue;
					break;
				case configSetting::channel :
					cvOutputs[midiEvent.cfgChannelOrOutput - 9].channel = midiEvent.configValue;
					break;
				case configSetting::controller :
					cvOutputs[midiEvent.cfgChannelOrOutput - 9].controller = midiEvent.configValue;
					break;
				}
			}
		}
	}

	// Controller
	if (midiEvent.msg == 0xB) {
		for (auto cv : cvOutputs) {
			if (cv.type == cvType::controller && cv.channel == midiEvent.chn + 1 && cv.controller == midiEvent.db1) {
				uint16_t dacOutput = 0xFFFF * (float)midiEvent.db2 / 128;		// controller values are from 0 to 127
				dacHandler.sendData(WriteChannel | ChannelA, dacOutput);
			}
		}
	}

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

	// Set pitch
	if (midiNotes.size() > 0) {
		//uint16_t dacOut = 4095 * (float)(std::min(std::max((int)midiNotes.back(), 24), 96) - 24) / 72;		// limit C1 to C7
		//DAC->DHR12R1 = dacOut;

		// Using external DAC
		uint16_t dacOutput = 0xFFFF * (float)(std::min(std::max((int)midiNotes.back(), 24), 96) - 24) / 72;		// limit C1 to C7
#ifdef MAX5134
		dacHandler.sendData(WriteChannel | ChannelA, dacOutput);
		dacHandler.sendData(WriteChannel | ChannelB, dacOutput);
#else
		dacHandler.sendData(WriteChannel, ChannelA, dacOutput);
		dacHandler.sendData(WriteChannel, ChannelB, dacOutput);
#endif
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

/*
Byte 1									|	Byte2		|	Byte 3		|	Byte 4
Cable Number | Code Index Number (CIN)	|	MIDI_0		|	MIDI_1		|	MIDI_2

CIN		MIDI_x Size Description
0x0		1, 2 or 3	Miscellaneous function codes. Reserved for future extensions.
0x1		1, 2 or 3	Cable events. Reserved for future expansion.
0x2		2			Two-byte System Common messages like MTC, SongSelect, etc.
0x3		3			Three-byte System Common messages like SPP, etc.
0x4		3			SysEx starts or continues
0x5		1			Single-byte System Common Message or SysEx ends with following single byte.
0x6		2			SysEx ends with following two bytes.
0x7		3			SysEx ends with following three bytes.
0x8		3			Note-off
0x9		3			Note-on
0xA		3			Poly-KeyPress
0xB		3			Control Change
0xC		2			Program Change
0xD		2			Channel Pressure
0xE		3			PitchBend Change
0xF		1			Single Byte
*/

