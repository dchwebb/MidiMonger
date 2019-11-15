#include <MidiHandler.h>

extern MidiData midiArray[MIDIBUFFERSIZE];

MidiHandler::MidiHandler() {
	setConfig();
}

void MidiHandler::setConfig() {
	// Calculate number of voices available for each channel
	uint8_t voices = 0;
	for (uint8_t c = 0; c < 16; ++c) {
		voices = 0;
		for (uint8_t v = 0; v < 4; ++v) {
			if (cvOutputs[v].channel == c + 1 && cvOutputs[v].type == cvType::channelPitch) {
				voices++;
			}
		}
		channelNotes[c].voiceCount = voices;
	}
	return;
}

void MidiHandler::serialHandler() {
	if (QueueSize > 0) {
		//bool edited = false;
		volatile uint8_t val1, val2;

		MIDIType type = static_cast<MIDIType>(Queue[QueueRead] >> 4);
		uint8_t channel = Queue[QueueRead] & 0x0F;

		//NoteOn = 0x9, NoteOff = 0x8, PolyPressure = 0xA, ControlChange = 0xB, ProgramChange = 0xC, ChannelPressure = 0xD, PitchBend = 0xE, System = 0xF
		while ((QueueSize > 2 && (type == NoteOn || type == NoteOff || type == PolyPressure ||  type == ControlChange ||  type == PitchBend)) ||
				(QueueSize > 1 && (type == ProgramChange || type == ChannelPressure))) {

			MidiData event;
			event.chn = channel;
			event.msg = (uint8_t)type;

			QueueInc();
			event.db1 = Queue[QueueRead];
			QueueInc();
			if (type == ProgramChange || type == ChannelPressure) {
				event.db2 = 0;
			} else {
				event.db2 = Queue[QueueRead];
				QueueInc();
			}

			eventHandler(event.data);

			type = static_cast<MIDIType>(Queue[QueueRead] >> 4);
			channel = Queue[QueueRead] & 0x0F;
		}

		// Clock
		if (QueueSize > 0 && Queue[QueueRead] == 0xF8) {
			ClockCount++;
			// MIDI clock triggers at 24 pulses per quarter note
			if (ClockCount == 6) {
				Clock = SysTickVal;
				ClockCount = 0;
			}
			QueueInc();
		}

		//	handle unknown data in queue
		if (QueueSize > 2 && type != 0x9 && type != 0x8 && type != 0xD && type != 0xE) {
			QueueInc();
		}
	}
}

inline void MidiHandler::QueueInc() {
	QueueSize--;
	QueueRead = (QueueRead + 1) % MIDIQUEUESIZE;
}

void MidiHandler::eventHandler(uint32_t data)
{

	MidiData midiEvent = MidiData(data);

	// Store Midi events to array for debugging
	/*midiArray[midiEventRead] = midiEvent;
	midiEventRead = midiEventRead == MIDIBUFFERSIZE - 1 ? 0 : midiEventRead + 1;*/

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
				default :
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
				default :
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

	//	Note on/note off
	if (midiEvent.msg == 9 || midiEvent.msg == 8) {
		// locate output that will process the request
		for (auto gate : gateOutputs) {
			if (gate.channel == midiEvent.chn + 1 && (gate.type == gateType::channelNote || (gate.type == gateType::specificNote && gate.note == midiEvent.db1))) {

				// Delete note if already playing and add to latest position in list
				activeNote& noteList = channelNotes[gate.channel - 1].activeNotes;
				noteList.remove(midiEvent.db1);
				if (midiEvent.msg == 9)
					noteList.push_back(midiEvent.db1);

				// work back through the active note list checking which voice to assign note to
				bool notePlaying;
				uint8_t noteToAssign = 0;			// stores note not currently assigned to a voice for allocation later
				auto currNote = noteList.cend();
				for (int8_t n = std::min((uint8_t)noteList.size(), channelNotes[gate.channel - 1].voiceCount); n > 0; n--) {
					currNote--;

					// check if any voice is currently playing note
					notePlaying = false;
					for (uint8_t c = 0; c < 4; ++c) {
						if (cvOutputs[c].currentNote == *currNote) {
							cvOutputs[c].nextNote = cvOutputs[c].currentNote;
							notePlaying = true;
							break;
						}
					}

					// if no voice is currently playing note it will be assigned after all playing notes are identified
					if (!notePlaying) {
						noteToAssign = *currNote;
					}
				}

				if (noteToAssign > 0) {
					for (uint8_t c = 0; c < 4; ++c) {
						if (cvOutputs[c].nextNote == 0) {
							cvOutputs[c].nextNote = noteToAssign;
							break;
						}
					}
				}

				// loop through all channels in group and update notes as required
				for (uint8_t c = 0; c < 4; ++c) {
					if (cvOutputs[c].nextNote != cvOutputs[c].currentNote) {
						// Mute
						if (cvOutputs[c].nextNote == 0) {
							cvOutputs[c].gpioPort->BSRR |= (1 << (16 + cvOutputs[c].gpioPin));			// Gate Off
						} else {
							uint16_t dacOutput = 0xFFFF * (float)(std::min(std::max((int)cvOutputs[c].nextNote, 24), 96) - 24) / 72;		// limit C1 to C7
							dacHandler.sendData(WriteChannel | cvOutputs[c].dacChannel, dacOutput);		// Send pitch to DAC
							cvOutputs[c].gpioPort->BSRR |= (1 << cvOutputs[c].gpioPin);					// Gate on
						}

					}
					cvOutputs[c].currentNote = cvOutputs[c].nextNote;
					cvOutputs[c].nextNote = 0;
				}

			}
		}

	}
	/*
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
		//dacHandler.sendData(WriteChannel | cvOutputs[1].dacChannel, dacOutput);
		//dacHandler.sendData(WriteChannel | ChannelB, dacOutput);
		//dacHandler.sendData(WriteChannel | ChannelC, dacOutput);
		//dacHandler.sendData(WriteChannel | ChannelD, dacOutput);

#else
		dacHandler.sendData(WriteChannel, ChannelA, dacOutput);
		dacHandler.sendData(WriteChannel, ChannelB, dacOutput);
#endif
	}

	// light up LED (PB14) and transmit gate (PA3)
	if (midiNotes.size() > 0) {
		GPIOB->BSRR |= GPIO_BSRR_BS_14;

		//cvOutputs[0].gpioPort->BSRR |= (1 << cvOutputs[1].gpioPin);
		//GPIOC->BSRR |= GPIO_BSRR_BS_3;		// Gate 5
		//GPIOC->BSRR |= GPIO_BSRR_BS_0;		// Gate 6
		//GPIOA->BSRR |= GPIO_BSRR_BS_3;		// Gate 7


	}
	else {
		GPIOB->BSRR |= GPIO_BSRR_BR_14;

		//cvOutputs[0].gpioPort->BSRR |= (1 << (16 + cvOutputs[1].gpioPin));
		//GPIOA->BSRR |= GPIO_BSRR_BR_3;		// Gate 7
		//GPIOC->BSRR |= GPIO_BSRR_BR_0;		// Gate 6
		//GPIOC->BSRR |= GPIO_BSRR_BR_3;		// Gate 5
	}*/

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

