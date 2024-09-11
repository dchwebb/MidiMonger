#include <MidiControl.h>
#include "DACHandler.h"
#include "USB.h"

MidiControl midiControl;

MidiControl::MidiControl()
{
	SetConfig();
}


void MidiControl::MidiEvent(const uint32_t data)
{
	auto midiEvent = MidiData(data);

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
				tx.cfgChannelOrOutput = gateOutputs[midiEvent.db1 - 1].channel - 1;
				tx.configValue = gateOutputs[midiEvent.db1 - 1].note;
			} else {
				// CV outputs (9 - 12)
				tx.configType = (uint8_t)cvOutputs[midiEvent.db1 - 9].type;
				tx.cfgChannelOrOutput = cvOutputs[midiEvent.db1 - 9].channel - 1;
				tx.configValue = cvOutputs[midiEvent.db1 - 9].controller;
			}

			if (hostMode) {

			} else {
				usb.midi.SendData((uint8_t*)&tx, 4);
			}
		} else {
			// configuration changed by editor format is ttttoooo vvvvvvvv where t is type of setting, o is output number (1-8 = gate, 9 - 12 = cv) and v is setting value
			if (midiEvent.cfgChannelOrOutput < 9) {
				// Gate configuration
				switch ((ConfigSetting)midiEvent.configType) {
				case ConfigSetting::type:
					cfg.gates[midiEvent.cfgChannelOrOutput - 1].type = (GateType)midiEvent.configValue;
					MatchChannelSetting(OutputType::gate, midiEvent.cfgChannelOrOutput - 1);
					break;
				case ConfigSetting::specificNote:
					cfg.gates[midiEvent.cfgChannelOrOutput - 1].note = midiEvent.configValue;
					break;
				case ConfigSetting::channel:
					cfg.gates[midiEvent.cfgChannelOrOutput - 1].channel = midiEvent.configValue;
					MatchChannelSetting(OutputType::gate, midiEvent.cfgChannelOrOutput - 1);
					break;
				default:
					break;
				}
			} else {																// CV Configuration
				switch ((ConfigSetting)midiEvent.configType) {
				case ConfigSetting::type:
					cfg.cvs[midiEvent.cfgChannelOrOutput - 9].type = (CvType)midiEvent.configValue;
					MatchChannelSetting(OutputType::cv, midiEvent.cfgChannelOrOutput - 9);
					break;
				case ConfigSetting::channel:
					cfg.cvs[midiEvent.cfgChannelOrOutput - 9].channel = midiEvent.configValue;
					MatchChannelSetting(OutputType::cv, midiEvent.cfgChannelOrOutput - 9);
					break;
				case ConfigSetting::controller:
					cfg.cvs[midiEvent.cfgChannelOrOutput - 9].controller = midiEvent.configValue;
					break;
				default:
					break;
				}
			}

			SetConfig();
			config.ScheduleSave();

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
			if (cv.type == CvType::controller && cv.channel == midiEvent.chn + 1 && cv.controller == midiEvent.db1) {
				uint16_t dacOutput = 0xFFFF * (float)midiEvent.db2 / 128;		// controller values are from 0 to 127
				dacHandler.SendData(DACHandler::WriteChannel | cv.dacChannel, dacOutput);
				cv.LedOn(200);
			}
		}
	}

	// Pitch Bend - 14 bit controller
	if (midiEvent.msg == PitchBend) {
		for (auto& cv : cvOutputs) {
			if (cv.type == CvType::channelPitch && cv.channel == midiEvent.chn + 1) {
				channelNotes[midiEvent.chn].pitchbend = (float)((midiEvent.db1 + (midiEvent.db2 << 7) - 8192) / 8192.0f) * cfg.pitchBendSemiTones;
				cv.SendNote();
			} else if (cv.type == CvType::pitchBend && cv.channel == midiEvent.chn + 1) {
				uint16_t dacOutput = (midiEvent.db1 + (midiEvent.db2 << 7)) << 2;		// convert 14 to 16 bit value
				dacHandler.SendData(DACHandler::WriteChannel | cv.dacChannel, dacOutput);
			}
		}
	}

	// Aftertouch
	if (midiEvent.msg == ChannelPressure) {
		for (auto& cv : cvOutputs) {
			if (cv.type == CvType::afterTouch && cv.channel == midiEvent.chn + 1) {
				uint16_t dacOutput = 0xFFFF * (float)midiEvent.db1 / 128;		// Aftertouch values are from 0 to 127
				dacHandler.SendData(DACHandler::WriteChannel | cv.dacChannel, dacOutput);	// Send pitch to DAC
				cv.LedOn(200);													// Turn LED On for 100ms
			}
		}
	}

	//	Note on/note off
	if (midiEvent.msg == NoteOn || midiEvent.msg == NoteOff) {
		//printf("Note: %d\r\n", midiEvent.db1);

		// locate output that will process the request
		for (auto& gate : gateOutputs) {
			if (gate.channel == midiEvent.chn + 1 && gate.type == GateType::specificNote && gate.note == midiEvent.db1) {
				if (midiEvent.msg == NoteOn) {
					gate.output.SetHigh();
				} else {
					gate.output.SetLow();
				}

			} else if (gate.channel == midiEvent.chn + 1 && gate.type == GateType::channelNote) {

				// Delete note if already playing and add to latest position in list
				activeNote& noteList = channelNotes[gate.channel - 1].activeNotes;
				noteList.remove(midiEvent.db1);
				if (midiEvent.msg == NoteOn) {
					noteList.push_back(midiEvent.db1);
				}

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
					if (!notePlaying && midiEvent.db1 == *currNote) {		// Added condition midiEvent.db1 == *currNote to prevent replaying notes that have been voice stolen
						noteToAssign = *currNote;
					}
				}

				if (noteToAssign > 0) {
					for (uint8_t c = 0; c < 4; ++c) {
						if (cvOutputs[c].nextNote == 0 && cvOutputs[c].channel == gate.channel && cvOutputs[c].type == CvType::channelPitch) {
							cvOutputs[c].nextNote = noteToAssign;
							break;
						}
					}
				}

				// loop through all channels in group and update notes as required
				for (uint8_t c = 0; c < 4; ++c) {
					if (cvOutputs[c].nextNote != cvOutputs[c].currentNote && cvOutputs[c].channel == gate.channel && cvOutputs[c].type == CvType::channelPitch) {
						// Mute
						if (cvOutputs[c].nextNote == 0) {
							gateOutputs[c].output.SetLow();
							cvOutputs[c].currentNote = 0;
						} else {
							cvOutputs[c].currentNote = cvOutputs[c].nextNote;
							cvOutputs[c].SendNote();
							gateOutputs[c].output.SetHigh();
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

		++ClockCount;

		// MIDI clock triggers at 24 pulses per quarter note
		if (ClockCount % 6 == 0) {
			for (auto& gate : gateOutputs) {
				if (gate.type == GateType::clock) {
					gate.GateOn(SysTickVal + 13);		// pass gate off time: each tick is around 400us: 15 x 400us = 6ms (less a bit from testing)
				}
			}
		}
	}
}


void MidiControl::SerialHandler(uint32_t data)
{
	queue[queueWrite] = data;
	queueCount++;
	queueWrite = (queueWrite + 1) % QueueSize;

	MIDIType type = static_cast<MIDIType>(queue[queueRead] >> 4);
	uint8_t channel = queue[queueRead] & 0x0F;

	//NoteOn = 0x9, NoteOff = 0x8, PolyPressure = 0xA, ControlChange = 0xB, ProgramChange = 0xC, ChannelPressure = 0xD, PitchBend = 0xE, System = 0xF
	while ((queueCount > 2 && (type == NoteOn || type == NoteOff || type == PolyPressure ||  type == ControlChange ||  type == PitchBend)) ||
			(queueCount > 1 && (type == ProgramChange || type == ChannelPressure))) {

		MidiData event;
		event.chn = channel;
		event.msg = (uint8_t)type;

		QueueInc();
		event.db1 = queue[queueRead];
		QueueInc();
		if (type == ProgramChange || type == ChannelPressure) {
			event.db2 = 0;
		} else {
			event.db2 = queue[queueRead];
			QueueInc();
		}

		MidiEvent(event.data);

		type = static_cast<MIDIType>(queue[queueRead] >> 4);
		channel = queue[queueRead] & 0x0F;
	}

	// Clock
	if (queueCount > 0 && queue[queueRead] == 0xF8) {
		MidiEvent(0xF800);
		QueueInc();
	}

	//	handle unknown data in queue
	if (queueCount > 2 && type != 0x9 && type != 0x8 && type != 0xD && type != 0xE) {
		QueueInc();
	}
}


inline void MidiControl::QueueInc()
{
	--queueCount;
	queueRead = (queueRead + 1) % QueueSize;
}


void MidiControl::GateTimer()
{
	//	Switches off clock ticks after specified time
	for (auto& gate : gateOutputs) {
		if (gate.gateOffTime > 0 && SysTickVal > gate.gateOffTime) {
			gate.output.SetLow();
			gate.gateOffTime = 0;
		}
	}
	for (auto& cv : cvOutputs) {
		if (cv.offTime > 0 && SysTickVal > cv.offTime) {
			cv.led.SetLow();
			cv.offTime = 0;
		}
	}
}


void MidiControl::GatesOff()
{
	for (auto& gate : gateOutputs) {		//	Switch off all gates
		gate.output.SetLow();
		gate.gateOffTime = 0;
	}
}


void MidiControl::SetConfig()
{
	// Validate and then load saved config settings
	for (uint32_t i = 0; i < 8; ++i) {
		auto& gate = midiControl.cfg.gates[i];
		if ((gate.type == GateType::channelNote && gate.channel > 0 && gate.channel <= 16) ||
			(gate.type == GateType::clock) ||
			(gate.type == GateType::specificNote && gate.note >= 24 && gate.note <= 96)) {
			midiControl.gateOutputs[i].type = gate.type;
			midiControl.gateOutputs[i].channel = gate.channel;
			midiControl.gateOutputs[i].note = gate.note;
		} else {								// If settings invalid, set to defaults
			gate.type = midiControl.gateOutputs[i].type;
			gate.channel = midiControl.gateOutputs[i].channel;
			gate.note = midiControl.gateOutputs[i].note;
		}
	}

	for (uint32_t i = 0; i < 4; ++i) {
		auto& cv = midiControl.cfg.cvs[i];
		if ((cv.type == CvType::channelPitch && cv.channel > 0 && cv.channel <= 16) ||
			(cv.type == CvType::controller && cv.controller < 128) ||
			(cv.type == CvType::pitchBend) || (cv.type == CvType::afterTouch)) {
			midiControl.cvOutputs[i].type = cv.type;
			midiControl.cvOutputs[i].channel = cv.channel;
			midiControl.cvOutputs[i].controller = cv.controller;
		} else {
			cv.type = midiControl.cvOutputs[i].type;
			cv.channel = midiControl.cvOutputs[i].channel;
			cv.controller = midiControl.cvOutputs[i].controller;
		}

		if (cv.type == CvType::pitchBend && dacHandler.ready) {			// Set pitchbend to mid level
			dacHandler.SendData(DACHandler::WriteChannel | midiControl.cvOutputs[i].dacChannel, 0x7FFF);
		}
	}

	// Calculate number of voices available for each channel
	uint8_t voices = 0;
	for (uint8_t c = 0; c < 16; ++c) {
		voices = 0;
		for (uint8_t v = 0; v < 4; ++v) {
			if (midiControl.cvOutputs[v].channel == c + 1 && midiControl.cvOutputs[v].type == CvType::channelPitch) {
				voices++;
			}
		}
		midiControl.channelNotes[c].voiceCount = voices;
	}


	// validate config settings
	if (midiControl.cfg.dacOffset < 0.00001f) {
		midiControl.cfg.dacOffset = dacOffsetDefault;
	}
	if (midiControl.cfg.dacScale < 0.00001f) {
		midiControl.cfg.dacScale = dacScaleDefault;
	}
	if (midiControl.cfg.pitchBendSemiTones < 0.00001f) {
		midiControl.cfg.pitchBendSemiTones = pitchBendSemiTonesDefault;
	}
}


void MidiControl::MatchChannelSetting(OutputType output, uint8_t num)
{
	// Ensures when setting cv channel pitch, gate channel is set to channel note and vice versa
	if (num > 3) {
		return;
	}

	auto& g = cfg.gates[num];
	auto& c = cfg.cvs[num];

	if (output == OutputType::gate) {
		// if switching gate to channel note ensure that cv is set to matching channel pitch
		if (num < 4 && g.type == GateType::channelNote && (c.type != CvType::channelPitch || c.channel != g.channel)) {
			c.type = CvType::channelPitch;
			c.channel = g.channel;
		}
	} else {		// CV
		// if switching cv to channel note ensure that gate is set to matching channel note
		if (num < 4 && c.type == CvType::channelPitch && (g.type != GateType::channelNote || g.channel != c.channel)) {
			g.type = GateType::channelNote;
			g.channel = c.channel;
		}
	}
}


void MidiControl::SendCV(uint16_t dacOutput, uint8_t channel)
{
	dacHandler.SendData(DACHandler::WriteChannel | cvOutputs[channel].dacChannel, dacOutput);		// Send pitch to DAC
	cvOutputs[channel].LedOn(200);																// Turn LED On for 200ms
}


void MidiControl::CV::SendNote()
{
	uint16_t dacOutput = 0xFFFF * (std::clamp((float)currentNote + midiControl.channelNotes[channel - 1].pitchbend, midiControl.cfg.dacOffset, 96.0f) - midiControl.cfg.dacOffset) / midiControl.cfg.dacScale;		// limit C1 to C7
	dacHandler.SendData(DACHandler::WriteChannel | dacChannel, dacOutput);		// Send pitch to DAC
	LedOn(400);																	// Turn LED On for 400ms
}


void MidiControl::LightShow()
{
	for (uint8_t c = 0; c < 8; ++c) {
		midiControl.cvOutputs[c > 3 ? 3 - c % 4 : c].LedOn(60);
		uint32_t delay = SysTickVal + 110;
		while (delay > SysTickVal) {
			midiControl.GateTimer();
		}
	}
}
