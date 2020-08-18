#include "CDCHandler.h"
extern MidiHandler midiHandler;
void LilLightShow();

int16_t ParseInt(const std::string cmd, const char precedingChar) {
	uint16_t val = -1;
	int8_t pos = cmd.find(precedingChar);		// locate position of character preceding
	if (pos >= 0 && std::strspn(cmd.substr(pos + 1).c_str(), "0123456789") > 0) {
		val = stoi(cmd.substr(pos + 1));
	}
	return val;
}

static std::string IntToStr(const int32_t& v) {
	std::stringstream ss;
	ss << v;
	return ss.str();
}

bool CDCCommand(const std::string ComCmd) {
	std::stringstream ss;
	bool changed = false;

	if (ComCmd.compare("help\n") == 0) {
		usb.SendString("Mountjoy MIDI Monger - supported commands:\n\n"
				"help      -  Shows this information\n"
				"config    -  Shows current control configuration\n"
				"kill      -  Kill all gates\n"
				"test      -  Light display\n"
				"nVcC      -  Play note value V on channel C. Eg \'n60c1\' is middle C on channel 1\n"
				"gGmMcCnN  -  Configure gate G (1-8) mode M (1=specific note, 2=channel gate, 3=clock)\n"
				"             Optional: channel C (1-16) note N (from 24=C1 to 96=C7)\n"
				"             Eg g7m1c10n37 to configure gate 7, mode Specific note, channel 10, note 37\n"
				"             Eg g2m2c12 to configure gate 2, mode Channel gate, channel 12\n"
				"             Eg g8m3 to configure gate 8, mode clock\n"
				"vCmMcCnN  -  Configure cv C (1-8) mode M (1=channel pitch, 2=controller, 3=pitchbend, 4=aftertouch)\n"
				"             Optional: channel C (1-16) controller N (0-127)\n"
				"             Eg v1m1c3 to configure cv 1, mode Channel Pitch, channel 3\n"
				"             Eg v3m2c12n7 to configure cv 3, mode controller, channel 12, controller 7\n"
				"pS        -  Set pitchbend range to S semitones Eg \'p2\' sets pitchbend range to 2 semitones\n"
		);


	} else 	if (ComCmd.compare(0, 1, "p") == 0) {			// Configure Pitchbend range

		int16_t pb = ParseInt(ComCmd, 'p');
		if (pb > 0 && pb < 25) {
			midiHandler.pitchBendSemiTones = pb;
			usb.SendString(("Pitchbend set to " + IntToStr(pb) + " semitones\n").c_str());
			changed = true;
		} else {
			return false;
		}

	} else 	if (ComCmd.compare(0, 1, "v") == 0) {			// Configure CVs

		int16_t cv = ParseInt(ComCmd, 'v');
		int16_t mode = ParseInt(ComCmd, 'm');
		int8_t channel = ParseInt(ComCmd, 'c');

		if (cv < 1 || cv > 4 || mode < 1 || mode > 4 || channel < 1 || channel > 16) {
			return false;
		}

		//cvType {channelPitch = 1, controller = 2, pitchBend = 3, afterTouch = 4}
		if ((cvType)mode == cvType::controller) {
			uint8_t controller = ParseInt(ComCmd, 'n');
			if (controller > 127) {
				return false;
			}
			midiHandler.cvOutputs[cv - 1].type = cvType::controller;
			midiHandler.cvOutputs[cv - 1].channel = channel;
			midiHandler.cvOutputs[cv - 1].controller = controller;
		} else if ((cvType)mode == cvType::channelPitch) {
			midiHandler.cvOutputs[cv - 1].type = cvType::channelPitch;
			midiHandler.cvOutputs[cv - 1].channel = channel;
			midiHandler.validateConfig(false, cv - 1);
		} else {
			midiHandler.cvOutputs[cv - 1].type = (cvType)mode;
			midiHandler.cvOutputs[cv - 1].channel = channel;
		}

		usb.SendString(("Configured: " + ComCmd + "\n").c_str());
		changed = true;

	} else 	if (ComCmd.compare(0, 1, "g") == 0) {			// Configure gates

		int16_t gate = ParseInt(ComCmd, 'g');
		int16_t mode = ParseInt(ComCmd, 'm');		// locate position of mode code

		if (gate < 1 || gate > 8 || mode < 1 || mode > 3) {
			return false;
		}

		//gateType {specificNote = 1, channelNote = 2, clock = 3};
		if ((gateType)mode == gateType::clock) {
			midiHandler.gateOutputs[gate - 1].type = gateType::clock;
		} else {
			int8_t channel = ParseInt(ComCmd, 'c');
			if (channel < 1 || channel > 16) {
				return false;
			}
			if ((gateType)mode == gateType::specificNote) {
				int8_t note = ParseInt(ComCmd, 'n');
				if (note < 24 || note > 96) {
					return false;
				}

				midiHandler.gateOutputs[gate - 1].type = gateType::specificNote;
				midiHandler.gateOutputs[gate - 1].channel = channel;
				midiHandler.gateOutputs[gate - 1].note = note;
			} else {
				midiHandler.gateOutputs[gate - 1].type = gateType::channelNote;
				midiHandler.gateOutputs[gate - 1].channel = channel;
				midiHandler.validateConfig(true, gate - 1);
			}
		}
		usb.SendString(("Configured: " + ComCmd + "\n").c_str());
		changed = true;


	} else if (ComCmd.compare("config\n") == 0) {			// Print current configuration

		ss << "Configuration:\n";
		uint16_t gate = 0;
		for (Gate& g : midiHandler.gateOutputs) {
			ss << "\nGate " << ++gate << ": ";
			switch(g.type) {
			case gateType::channelNote:
				ss << "Channel Gate. Channel: " << (int)g.channel;
				break;
			case gateType::specificNote:
				ss << "Specific Note. Channel: " << ((int)g.channel) << " Note: " << (int)g.note;
				break;
			case gateType::clock: ss << "Clock"; break;
			}
		}

		ss << '\n';
		uint16_t c = 0;
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
		ss << "\n\nPitchbend range: " + IntToStr(midiHandler.pitchBendSemiTones) + " semitones\n";
		usb.SendString(ss.str().c_str());

	} else if (ComCmd.compare("test\n") == 0) {
		LilLightShow();

	} else if (ComCmd.compare("kill\n") == 0) {
		midiHandler.gatesOff();

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
			return false;
		}
	} else {
		return false;
	}
	if (changed) {
		midiHandler.setConfig();
		cfg.ScheduleSave();
	}
	return true;
}
