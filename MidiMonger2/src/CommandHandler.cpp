#include "CommandHandler.h"
#include "USB.h"
#include "uartHandler.h"
#include "MidiControl.h"
#include "HidDescriptor.h"
#include "HidHostClass.h"
#include "USBHost.h"

CommandHandler commandHandler;

// Capture buffer for printing USB host logging over USB device connection
static constexpr uint32_t logBufSize = 20000;
uint32_t logBufPos = 0;
char logBuf[logBufSize];

extern "C" {
size_t _write(int handle, const unsigned char* buf, size_t len)
{
	uart.SendString(buf, len);									// Logging via UART
	if (hostMode && logBufPos < logBufSize && (char*)buf != logBuf) {					// Capture host mode logging for later printing out over device mode
		uint32_t copyLen = logBufPos + len > logBufSize ? logBufSize - logBufPos : len;
		memcpy(&logBuf[logBufPos], buf, copyLen);
		logBufPos += copyLen;
	}

	if (usb.devState == USB::DeviceState::Configured) {			// Logging via USB
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

	if (cmd.starts_with("help")) {
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
				"pb:S        -  Set pitchbend range to S semitones Eg \'p2\' sets pitchbend range to 2 semitones\r\n"
				"porta:N     -  Set portamento amount to N for monophonic channels (0-255)\r\n"
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
				"\r\n"
				"mcC:N       -  Configure mouse control X/Y/wheel: control c (X,Y,W) CV output N (1-4) \r\n"
				"               Eg mcW:2 to output mouse wheel to CV 2\r\n"
				"\r\n"
				"mbB:N       -  Configure mouse button B (1-8) Gate output N (1-8) \r\n"
				"               Eg mb2:7 to output mouse button 2 to Gate 7\r\n"
		);



	} else 	if (cmd.starts_with("dacoffset:")) {			// Configure DAC offset
		float dacOffset = ParseFloat(cmd, ':', 10, 30);
		if (dacOffset > 0) {
			midiControl.cfg.dacOffset = dacOffset;
			printf("DAC offset set to %f\r\n", dacOffset);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}


	} else 	if (cmd.starts_with("dacscale:")) {				// Configure DAC scale
		float dacScale = ParseFloat(cmd, ':', 50, 100);
		if (dacScale > 0) {
			midiControl.cfg.dacScale = dacScale;
			printf("DAC scale set to %f\r\n", dacScale);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}


	} else 	if (cmd.starts_with("pb:")) {					// Configure Pitchbend range
		int16_t pb = ParseInt(cmd, ':', 0, 25);
		if (pb > 0) {
			midiControl.cfg.pitchBendSemiTones = pb;
			printf("Pitchbend set to %d semitones\r\n", pb);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}


	} else 	if (cmd.starts_with("porta:")) {				// Configure portamento amount
		int16_t p = ParseInt(cmd, ':', 0, 255);
		if (p >= 0) {
			midiControl.cfg.portamento = p;
			printf("Portamento set to %d\r\n", p);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}


	} else 	if (cmd.starts_with("mb")) {					// Mouse buttons (Eg mb2:7 to output mouse button 2 to Gate 7)
		int16_t button = ParseInt(cmd, 'b', 1, 8);
		int16_t gate = ParseInt(cmd, ':', 1, 8);
		if (button && gate) {
			hidHostClass.cfg.gateSource[gate - 1] = (HidHostClass::GateSource)button;
			printf("Configured: %s\r\n", cmd.data());
			config.ScheduleSave();
		}


	} else 	if (cmd.starts_with("mc")) {					// Mouse control (Eg mcW:2 to output mouse wheel to CV 2)
		auto control =	cmd[2] == 'X' ? HidHostClass::CVSource::mouseX :
						cmd[2] == 'Y' ? HidHostClass::CVSource::mouseY :
						cmd[2] == 'W' ? HidHostClass::CVSource::mouseWheel :
						HidHostClass::CVSource::noCV;
		int16_t cv = ParseInt(cmd, ':', 1, 8);
		if (control && cv) {
			hidHostClass.cfg.cvSource[cv - 1] = control;
			printf("Configured: %s\r\n", cmd.data());
			config.ScheduleSave();
		}


	} else 	if (cmd.starts_with("v")) {						// Configure CVs
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


	} else if (cmd.starts_with("g")) {					// Configure gates
		int16_t gate = ParseInt(cmd, 'g') - 1;
		int16_t mode = ParseInt(cmd, 'm');
		if (gate < 0 || gate > 7 || mode < 1 || mode > 3) {
			return;
		}

		// gateType {specificNote = 1, channelNote = 2, clock = 3};
		if ((MidiControl::GateType)mode == MidiControl::GateType::clock) {
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


	} else if (cmd.starts_with("info")) {					// Print current configuration
		auto buffPos = buf;
		buffPos += sprintf(buffPos, "Configuration:\r\n"
#ifdef V1_HARDWARE
				"V1 Hardware\r\n"
#endif
				"DAC offset: %f\r\n"
				"DAC scale: %f\r\n"
				"Pitchbend range: %f semitones\r\n"
				"Portamento amount: %d\r\n"
				"Config sector: %lu; address: %p\r\n",
				midiControl.cfg.dacOffset,
				midiControl.cfg.dacScale,
				midiControl.cfg.pitchBendSemiTones,
				midiControl.cfg.portamento,
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
#ifndef V1_HARDWARE
			buffPos += sprintf(buffPos, " [Mouse Button: %d]", hidHostClass.cfg.gateSource[gate - 1]);
#endif
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

#ifndef V1_HARDWARE
			auto src = hidHostClass.cfg.cvSource[c - 1];
			buffPos += sprintf(buffPos, " [Mouse: %s]",
					src == HidHostClass::CVSource::mouseX ? "X" :
					src == HidHostClass::CVSource::mouseY ? "Y" :
					src == HidHostClass::CVSource::mouseWheel ? "Wheel" : "");
#endif
		}

		buffPos += sprintf(buffPos, "\r\n\r\n");
		printf(buf);


	} else if (cmd.starts_with("lights")) {
		midiControl.LightShow(MidiControl::LightShowType::connection);


	} else if (cmd.starts_with("kill")) {
		midiControl.GatesOff();


	} else 	if (cmd.starts_with("n")) {	// Play note: Eg 'n60c1' is middle C on channel 1
		int8_t cpos = cmd.find("c");
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


	} else if (cmd.starts_with("saveconfig")) {					// Immediate config save
		config.SaveConfig(true);


	} else if (cmd.starts_with("clearconfig")) {				// Erase config from internal flash
		printf("Clearing config and restarting ...\r\n");
		config.EraseConfig();
		DelayMS(10);
		Reboot();


	} else if (cmd.starts_with("eventlistraw")) {					// Output MIDI events debug
		for (uint32_t i = 0; i < midiControl.midiDebugCount; ++i) {
			printf("%#010lx\r\n", midiControl.debugEvents[i].data);
		}

	} else if (cmd.starts_with("eventlist")) {					// Output MIDI events debug

		printf("CIN cable chn msg  db1 db2 Raw\r\n");
		for (uint32_t i = 0; i < midiControl.midiDebugCount; ++i) {
			printf("%3d %5d %3d %#04lx %3d %3d %#010lx\r\n",
					midiControl.debugEvents[i].CIN,
					midiControl.debugEvents[i].cable,
					midiControl.debugEvents[i].chn,
					(uint32_t)midiControl.debugEvents[i].msg,
					midiControl.debugEvents[i].db1,
					midiControl.debugEvents[i].db2,
					midiControl.debugEvents[i].data);
		}
		extern uint32_t debugMIDImultiEvents, debugMIDIsplitEvents;
		printf("Filtered events: %lu; Multi data events: %lu; Split events: %lu\r\n", midiControl.midiDebugFilterCount, debugMIDImultiEvents, debugMIDIsplitEvents);


	} else if (cmd.starts_with("hostlog")) {					// Output host log
		_write(0, (const unsigned char*)logBuf, logBufPos);



	} else if (cmd.starts_with("disableusb")) {					// Disable USB device
		usb.Disable();

#if (USB_DEBUG)
	} else if (cmd.starts_with("usbdebug")) {					// Debug USB
		if (!hostMode) {
			usb.OutputDebug();
		}
#endif

	} else {
		printf("Unrecognised command. Type 'help' for supported commands\r\n");
	}

	if (changed) {
		midiControl.SetConfig();
		config.ScheduleSave();
	}
}


int32_t CommandHandler::ParseInt(const std::string_view cmd, const char precedingChar, const int32_t low, const int32_t high)
{
	int32_t val = -1;
	const int8_t pos = cmd.find(precedingChar);
	if (pos >= 0 && std::strspn(&cmd[pos + 1], "0123456789-") > 0) {
		val = std::stoi(&cmd[pos + 1]);
	}
	if (high > low && (val > high || val < low)) {
		printf("Must be a value between %ld and %ld\r\n", low, high);
		return low - 1;
	}
	return val;
}


float CommandHandler::ParseFloat(const std::string_view cmd, const char precedingChar, const float low = 0.0f, const float high = 0.0f)
{
	float val = -1.0f;
	const int8_t pos = cmd.find(precedingChar);
	if (pos >= 0 && std::strspn(&cmd[pos + 1], "0123456789.") > 0) {
		val = std::stof(&cmd[pos + 1]);
	}
	if (high > low && (val > high || val < low)) {
		printf("Must be a value between %f and %f\r\n", low, high);
		return -1.0f;
	}
	return val;
}
