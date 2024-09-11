#include "CommandHandler.h"
#include "USB.h"
#include "uartHandler.h"
#include "MidiControl.h"
#include "HidDescriptor.h"

CommandHandler commandHandler;

extern "C" {
size_t _write(int handle, const unsigned char* buf, size_t len)
{
	uart.SendString(buf, len);									// Logging via UART

	if (usb.devState == USB::DeviceState::Configured) {		// Logging via USB
		usb.SendString(buf, len);
	}

	return len;
}
}


void CommandHandler::CheckCommands()
{
	usb.cdc.ProcessCommand();
	uart.ProcessCommand();
}


void CommandHandler::ProcessCommand(std::string_view cmd)
{
	bool changed = false;

	if (cmd.compare("help") == 0) {
		printf("Mountjoy MIDI Monger - supported commands:\r\n\r\n"
				"help        -  Shows this information\r\n"
				"info        -  Shows current control configuration\r\n"
				"dfu         -  USB firmware upgrade\r\n"
				"kill        -  Kill all gates\r\n"
				"lights      -  Light display\r\n"
				"dacoffset:xx.x Starting pitch (default 25.3)\r\n"
				"dacscale:xx.x  Pitch scaling (default 74.0)\r\n"
				"clearconfig -  Erase configuration and restart\r\n"
				"saveconfig  -  Immediately save config\r\n"
				"nVcC        -  Play note value V on channel C. Eg \'n60c1\' is middle C on channel 1\r\n"
				"pS          -  Set pitchbend range to S semitones Eg \'p2\' sets pitchbend range to 2 semitones\r\n"
				"\r\n"
				"gGmMcCnN    -  Configure gate G (1-8) mode M (1=specific note, 2=channel gate, 3=clock)\r\n"
				"               Optional: channel C (1-16) note N (from 24=C1 to 96=C7)\r\n"
				"               Eg g7m1c10n37 to configure gate 7, mode Specific note, channel 10, note 37\r\n"
				"               Eg g2m2c12 to configure gate 2, mode Channel gate, channel 12\r\n"
				"               Eg g8m3 to configure gate 8, mode clock\r\n"
				"\r\n"
				"vCmMcCnN    -  Configure cv C (1-8) mode M (1=channel pitch, 2=controller, 3=pitchbend, 4=aftertouch)\r\n"
				"               Optional: channel C (1-16) controller N (0-127)\r\n"
				"               Eg v1m1c3 to configure cv 1, mode Channel Pitch, channel 3\r\n"
				"               Eg v3m2c12n7 to configure cv 3, mode controller, channel 12, controller 7\r\n"
		);


	} else 	if (cmd.compare(0, 10, "dacoffset:") == 0) {	// Configure DAC offset
		float dacOffset = ParseFloat(cmd, ':', 10, 30);
		if (dacOffset > 0) {
			midiControl.cfg.dacOffset = dacOffset;
			printf("DAC offset set to %f\r\n", dacOffset);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}

	} else 	if (cmd.compare(0, 9, "dacscale:") == 0) {		// Configure DAC scale
		float dacScale = ParseFloat(cmd, ':', 50, 100);
		if (dacScale > 0) {
			midiControl.cfg.dacScale = dacScale;
			printf("DAC scale set to %f\r\n", dacScale);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}

	} else 	if (cmd.compare(0, 1, "p") == 0) {				// Configure Pitchbend range
		int16_t pb = ParseInt(cmd, 'p', 0, 25);
		if (pb > 0) {
			midiControl.cfg.pitchBendSemiTones = pb;
			printf("Pitchbend set to %d semitones\r\n", pb);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}


	} else 	if (cmd.compare(0, 1, "v") == 0) {				// Configure CVs
		int16_t cv = ParseInt(cmd, 'v') - 1;
		int16_t mode = ParseInt(cmd, 'm');
		int8_t channel = ParseInt(cmd, 'c');
		if (cv < 0 || cv > 3 || mode < 1 || mode > 4 || channel < 1 || channel > 16) {
			return;
		}

		// cvType {channelPitch = 1, controller = 2, pitchBend = 3, afterTouch = 4}
		if ((MidiControl::CvType)mode == MidiControl::CvType::controller) {
			uint8_t controller = ParseInt(cmd, 'n');
			if (controller > 127) {
				return;
			}
			midiControl.cfg.cvs[cv].type = MidiControl::CvType::controller;
			midiControl.cfg.cvs[cv].channel = channel;
			midiControl.cfg.cvs[cv].controller = controller;
		} else if ((MidiControl::CvType)mode == MidiControl::CvType::channelPitch) {
			midiControl.cfg.cvs[cv].type = MidiControl::CvType::channelPitch;
			midiControl.cfg.cvs[cv].channel = channel;
			midiControl.MatchChannelSetting(MidiControl::OutputType::cv, cv);
		} else {
			midiControl.cfg.cvs[cv].type = (MidiControl::CvType)mode;
			midiControl.cfg.cvs[cv].channel = channel;
		}
		printf("Configured: %s\r\n", cmd.data());
		changed = true;


	} else 	if (cmd.compare(0, 1, "g") == 0) {				// Configure gates
		int16_t gate = ParseInt(cmd, 'g') - 1;
		int16_t mode = ParseInt(cmd, 'm');					// locate position of mode code
		if (gate < 0 || gate > 7 || mode < 1 || mode > 3) {
			return;
		}

		if ((MidiControl::GateType)mode == MidiControl::GateType::clock) {		// gateType {specificNote = 1, channelNote = 2, clock = 3};
			midiControl.cfg.gates[gate].type = MidiControl::GateType::clock;
		} else {
			int8_t channel = ParseInt(cmd, 'c');
			if (channel < 1 || channel > 16) {
				return;
			}
			if ((MidiControl::GateType)mode == MidiControl::GateType::specificNote) {
				int8_t note = ParseInt(cmd, 'n');
				if (note < 24 || note > 96) {
					return;
				}

				midiControl.cfg.gates[gate].type = MidiControl::GateType::specificNote;
				midiControl.cfg.gates[gate].channel = channel;
				midiControl.cfg.gates[gate].note = note;
			} else {
				midiControl.cfg.gates[gate].type = MidiControl::GateType::channelNote;
				midiControl.cfg.gates[gate].channel = channel;
				midiControl.MatchChannelSetting(MidiControl::OutputType::gate, gate);
			}
		}
		printf("Configured: %s\r\n", cmd.data());
		changed = true;


	} else if (cmd.compare("info") == 0) {					// Print current configuration
		auto buffPos = buf;
		buffPos += sprintf(buffPos, "Configuration:\r\n"
#ifdef V1_HARDWARE
				"V1 Hardware\r\n"
#endif
				"DAC offset: %f\r\n"
				"DAC scale: %f\r\n"
				"Pitchbend range: %f semitones\r\n"
				"Config sector: %lu; address: %p\r\n",
				midiControl.cfg.dacOffset,
				midiControl.cfg.dacScale,
				midiControl.cfg.pitchBendSemiTones,
				config.currentSector,
				config.flashConfigAddr + config.currentSettingsOffset / 4);

		uint16_t gate = 0;
		for (auto& g : midiControl.gateOutputs) {
			buffPos += sprintf(buffPos, "\r\nGate %d: ", ++gate);
			switch(g.type) {
			case MidiControl::GateType::channelNote:
				buffPos += sprintf(buffPos, "Channel Gate. Channel: %d", g.channel);
				break;
			case MidiControl::GateType::specificNote:
				buffPos += sprintf(buffPos, "Specific Note. Channel: %d Note: %d ", g.channel, g.note);
				break;
			case MidiControl::GateType::clock:
				buffPos += sprintf(buffPos, "Clock");
				break;
			}
		}
		buffPos += sprintf(buffPos, "\r\n");

		uint32_t c = 0;
		for (auto& cv : midiControl.cvOutputs) {
			buffPos += sprintf(buffPos, "\r\nCV %lu: ", ++c);
			switch (cv.type) {
			case MidiControl::CvType::channelPitch:
				buffPos += sprintf(buffPos, "Channel Pitch");
				break;
			case MidiControl::CvType::controller:
				buffPos += sprintf(buffPos, "Controller: %d", cv.controller);
				break;
			case MidiControl::CvType::pitchBend:
				buffPos += sprintf(buffPos, "Pitchbend");
				break;
			case MidiControl::CvType::afterTouch:
				buffPos += sprintf(buffPos, "Aftertouch");
				break;
			}
			buffPos += sprintf(buffPos, ". Channel: %d", cv.channel);
		}
		buffPos += sprintf(buffPos, "\r\n\r\n");
		printf(buf);


	} else if (cmd.compare("lights") == 0) {
		midiControl.LightShow();


	} else if (cmd.compare("kill") == 0) {
		midiControl.GatesOff();


	} else 	if (cmd.compare(0, 1, "n") == 0) {			// Play note: Eg 'n60c1' is middle C on channel 1
		int8_t cpos = cmd.find("c");					// locate position of channel code
		// Check that command is correctly formed - note that stoi can throw exceptions which breaks program
		if (cpos > 1 && std::strspn(&cmd[1], "0123456789") > 0 && std::strspn(&cmd[cpos + 1], "0123456789") > 0) {
			uint8_t noteVal = std::stoi(cmd.substr(1, cpos).data());
			std::string s = cmd.substr(cpos + 1).data();
			uint8_t channel = std::stoi(cmd.substr(cpos + 1).data());

			MidiControl::MidiData midiEvent;
			midiEvent.chn = channel - 1;
			midiEvent.msg = MidiControl::NoteOn;
			midiEvent.db1 = noteVal;
			midiControl.MidiEvent(midiEvent.data);

			DelayMS(1000);

			midiEvent.msg = MidiControl::NoteOff;
			midiControl.MidiEvent(midiEvent.data);
		}


	} else if (cmd.compare("saveconfig") == 0) {				// Immediate config save
		config.SaveConfig(true);


	} else if (cmd.compare("clearconfig") == 0) {				// Erase config from internal flash
		printf("Clearing config and restarting ...\r\n");
		config.EraseConfig();
		DelayMS(10);
		Reboot();


	} else if (cmd.compare("disableusb") == 0) {				// Disable USB device
		usb.Disable();


	} else {
		printf("Unrecognised command. Type 'help' for supported commands\r\n");
	}

	if (changed) {
		midiControl.SetConfig();
		config.ScheduleSave();
	}
}

int32_t CommandHandler::ParseInt(const std::string_view cmd, const char precedingChar, const int32_t low, const int32_t high) {
	int32_t val = -1;
	const int8_t pos = cmd.find(precedingChar);		// locate position of character preceding
	if (pos >= 0 && std::strspn(&cmd[pos + 1], "0123456789-") > 0) {
		val = std::stoi(&cmd[pos + 1]);
	}
	if (high > low && (val > high || val < low)) {
		printf("Must be a value between %ld and %ld\r\n", low, high);
		return low - 1;
	}
	return val;
}


float CommandHandler::ParseFloat(const std::string_view cmd, const char precedingChar, const float low = 0.0f, const float high = 0.0f) {
	float val = -1.0f;
	const int8_t pos = cmd.find(precedingChar);		// locate position of character preceding
	if (pos >= 0 && std::strspn(&cmd[pos + 1], "0123456789.") > 0) {
		val = std::stof(&cmd[pos + 1]);
	}
	if (high > low && (val > high || val < low)) {
		printf("Must be a value between %f and %f\r\n", low, high);
		return -1.0f;
	}
	return val;
}
