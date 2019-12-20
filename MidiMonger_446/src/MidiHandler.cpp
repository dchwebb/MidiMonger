#include "MidiHandler.h"

//extern MidiData midiArray[MIDIBUFFERSIZE];		// for debugging
channelNote MidiHandler::channelNotes[16] = {};		// definition of static declared array

void MidiHandler::CV::sendNote() {
	uint16_t dacOutput = 0xFFFF * (float)(std::min(std::max((float)currentNote + channelNotes[channel - 1].pitchbend, 24.0f), 96.0f) - 24) / 72;		// limit C1 to C7
	dacHandler.sendData(WriteChannel | dacChannel, dacOutput);		// Send pitch to DAC
	ledOn(400);										// Turn LED On for 400ms
}

void MidiHandler::CV::cvInit() {
	gpioPort->MODER |= (1 << (2 * gpioPin));		// Set to output
}

void MidiHandler::CV::ledOn(float offMilliseconds) {
	gpioPort->BSRR |= (1 << gpioPin);				// LED on
	offTime = SysTickVal + (offMilliseconds * 2.5);					// pass gate off time: each tick is around 400us: 2500 x 400us = 1 second
}

MidiHandler::MidiHandler() {
	// Init Hardware
	InitIO();
	for (Gate& g : gateOutputs) {
		g.gateInit();
	}
	for (CV& cv : cvOutputs) {
		cv.cvInit();
	}
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

// Overload to check if configuration is valid
void MidiHandler::validateConfig(bool isGate, uint8_t num) {
	if (num > 3)
		return;

	Gate& g = gateOutputs[num];
	CV& c = cvOutputs[num];

	if (isGate) {
		// if switching gate to channel note ensure that cv is set to matching channel pitch
		if (num < 4 && g.type == gateType::channelNote && (c.type != cvType::channelPitch || c.channel != g.channel)) {
			c.type = cvType::channelPitch;
			c.channel = g.channel;
		}
	} else {		// CV
		// if switching gate to channel note ensure that cv is set to matching channel pitch
		if (num < 4 && c.type == cvType::channelPitch && (g.type != gateType::channelNote || g.channel != c.channel)) {
			g.type = gateType::channelNote;
			g.channel = c.channel;
		}

	}
}

void MidiHandler::serialHandler(uint32_t data) {
	Queue[QueueWrite] = data;
	QueueSize++;
	QueueWrite = (QueueWrite + 1) % MIDIQUEUESIZE;

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
		eventHandler(0xF800);
		QueueInc();
	}

	//	handle unknown data in queue
	if (QueueSize > 2 && type != 0x9 && type != 0x8 && type != 0xD && type != 0xE) {
		QueueInc();
	}
}

inline void MidiHandler::QueueInc() {
	QueueSize--;
	QueueRead = (QueueRead + 1) % MIDIQUEUESIZE;
}

void MidiHandler::eventHandler(const uint32_t& data)
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
					validateConfig(true, midiEvent.cfgChannelOrOutput - 1);
					break;
				case configSetting::specificNote :
					gateOutputs[midiEvent.cfgChannelOrOutput - 1].note = midiEvent.configValue;
					break;
				case configSetting::channel :
					gateOutputs[midiEvent.cfgChannelOrOutput - 1].channel = midiEvent.configValue;
					validateConfig(true, midiEvent.cfgChannelOrOutput - 1);
					break;
				default :
					break;
				}
			} else {
				// CV Configuration
				switch ((configSetting)midiEvent.configType) {
				case configSetting::type :
					cvOutputs[midiEvent.cfgChannelOrOutput - 9].type = (cvType)midiEvent.configValue;
					validateConfig(false, midiEvent.cfgChannelOrOutput - 9);
					break;
				case configSetting::channel :
					cvOutputs[midiEvent.cfgChannelOrOutput - 9].channel = midiEvent.configValue;
					validateConfig(false, midiEvent.cfgChannelOrOutput - 9);
					break;
				case configSetting::controller :
					cvOutputs[midiEvent.cfgChannelOrOutput - 9].controller = midiEvent.configValue;
					break;
				default :
					break;
				}
			}

			setConfig();
			cfg.ScheduleSave();

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

	// Controller
	if (midiEvent.msg == ControlChange) {
		for (auto& cv : cvOutputs) {
			if (cv.type == cvType::controller && cv.channel == midiEvent.chn + 1 && cv.controller == midiEvent.db1) {
				uint16_t dacOutput = 0xFFFF * (float)midiEvent.db2 / 128;		// controller values are from 0 to 127
				dacHandler.sendData(WriteChannel | cv.dacChannel, dacOutput);
				cv.ledOn(200);													// Turn LED On for 100ms
			}
		}
	}

	// Pitch Bend
	if (midiEvent.msg == PitchBend) {
		for (auto& cv : cvOutputs) {
			if (cv.type == cvType::channelPitch && cv.channel == midiEvent.chn + 1) {
				channelNotes[midiEvent.chn].pitchbend = (float)((midiEvent.db1 + (midiEvent.db2 << 7) - 8192) / 8192.0f) * (float)pitchBendSemiTones;
				cv.sendNote();
			}
		}
	}

	// Aftertouch
	if (midiEvent.msg == ChannelPressure) {
		for (auto& cv : cvOutputs) {
			if (cv.type == cvType::afterTouch && cv.channel == midiEvent.chn + 1) {
				uint16_t dacOutput = 0xFFFF * (float)midiEvent.db1 / 128;		// Aftertouch values are from 0 to 127
				dacHandler.sendData(WriteChannel | cv.dacChannel, dacOutput);	// Send pitch to DAC
				cv.ledOn(200);													// Turn LED On for 100ms
			}
		}
	}

	//	Note on/note off
	if (midiEvent.msg == NoteOn || midiEvent.msg == NoteOff) {
		// locate output that will process the request
		for (auto& gate : gateOutputs) {
			if (gate.channel == midiEvent.chn + 1 && gate.type == gateType::specificNote && gate.note == midiEvent.db1) {
				if (midiEvent.msg == 9)
					gate.gateOn();
				else
					gate.gateOff();

			} else if (gate.channel == midiEvent.chn + 1 && gate.type == gateType::channelNote) {

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
						if (cvOutputs[c].currentNote == *currNote && cvOutputs[c].channel == gate.channel) {
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
						if (cvOutputs[c].nextNote == 0 && cvOutputs[c].channel == gate.channel && cvOutputs[c].type == cvType::channelPitch) {
							cvOutputs[c].nextNote = noteToAssign;
							break;
						}
					}
				}

				// loop through all channels in group and update notes as required
				for (uint8_t c = 0; c < 4; ++c) {
					if (cvOutputs[c].nextNote != cvOutputs[c].currentNote && cvOutputs[c].channel == gate.channel && cvOutputs[c].type == cvType::channelPitch) {
						// Mute
						if (cvOutputs[c].nextNote == 0) {
							gateOutputs[c].gateOff();
							cvOutputs[c].currentNote = 0;
						} else {
							cvOutputs[c].currentNote = cvOutputs[c].nextNote;
							cvOutputs[c].sendNote();
							gateOutputs[c].gateOn();
						}
					}
					cvOutputs[c].nextNote = 0;
				}

				break;		// if any polyphonic note found exit
			}
		}
	}

	// Clock
	if (midiEvent.db0 == 0xF8) {

		ClockCount++;

		// MIDI clock triggers at 24 pulses per quarter note
		if (ClockCount % 6 == 0) {
			for (auto& gate : gateOutputs) {
				if (gate.type == gateType::clock) {
					gate.gateOn(SysTickVal + 13);		// pass gate off time: each tick is around 400us: 15 x 400us = 6ms (less a bit from testing)
				}
			}
		}
	}

}

//	Switches off clock ticks after specified time
void MidiHandler::gateTimer() {
	for (auto& gate : gateOutputs) {
		if (gate.gateOffTime > 0 && SysTickVal > gate.gateOffTime) {
			gate.gateOff();
			gate.gateOffTime = 0;
		}
	}
	for (auto& cv : cvOutputs) {
		if (cv.offTime > 0 && SysTickVal > cv.offTime) {
			cv.gpioPort->BSRR |= (1 << (16 + cv.gpioPin));			// LED Off
			cv.offTime = 0;
		}
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

