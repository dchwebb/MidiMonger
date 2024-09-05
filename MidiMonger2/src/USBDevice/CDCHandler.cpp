#include "USB.h"
#include "CDCHandler.h"
#include "MidiControl.h"

void LilLightShow();

void CDCHandler::ProcessCommand()
{
	if (!cmdPending) {
		return;
	}

	bool changed = false;
	std::string_view cmd {comCmd};

	// Provide option to switch to USB DFU mode - this allows the MCU to be programmed with STM32CubeProgrammer in DFU mode
	if (state == serialState::dfuConfirm) {
		if (cmd.compare("y") == 0 || cmd.compare("Y") == 0) {
			usb->SendString("Switching to DFU Mode ...\r\n");
			uint32_t old = SysTickVal;
			while (SysTickVal < old + 100) {};		// Give enough time to send the message
			JumpToBootloader();
		} else {
			state = serialState::pending;
			usb->SendString("Upgrade cancelled\r\n");
		}
	} else if (cmd.compare("help") == 0) {
		usb->SendString("Mountjoy MIDI Monger - supported commands:\r\n\r\n"
				"help        -  Shows this information\r\n"
				"info        -  Shows current control configuration\r\n"
				"dfu         -  USB firmware upgrade\r\n"
				"kill        -  Kill all gates\r\n"
				"lights      -  Light display\r\n"
				"dacoffset:xx.x Starting pitch (default 25.3)\r\n"
				"dacScale:xx.x  PItch scaling (default 74.0)\r\n"
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

	} else if (cmd.compare("dfu") == 0) {					// USB DFU firmware upgrade
		printf("Start DFU upgrade mode? Press 'y' to confirm.\r\n");
		state = serialState::dfuConfirm;

	} else 	if (cmd.compare(0, 10, "dacoffset:") == 0) {	// Configure DAC offset
		float dacOffset = ParseFloat(cmd, ':', 10, 30);
		if (dacOffset > 0) {
			midiControl.cfg.dacOffset = dacOffset;
			printf("DAC offset set to %f\r\n", dacOffset);
			changed = true;
		} else {
			printf("Invalid range\r\n");
		}

	} else 	if (cmd.compare(0, 10, "dacscale:") == 0) {		// Configure DAC scale
		float dacScale = ParseFloat(cmd, ':', 10, 30);
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
		usb->SendString(buf);


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
		usb->Disable();


	} else {
		usb->SendString("Unrecognised command. Type 'help' for supported commands\r\n");
	}

	if (changed) {
		midiControl.SetConfig();
		config.ScheduleSave();
	}

	cmdPending = false;
}


void CDCHandler::DataIn()
{
	if (inBuffSize > 0 && inBuffSize % USB::ep_maxPacket == 0) {
		inBuffSize = 0;
		EndPointTransfer(Direction::in, inEP, 0);				// Fixes issue transmitting an exact multiple of max packet size (n x 64)
	}
}


// As this is called from an interrupt assign the command to a variable so it can be handled in the main loop
void CDCHandler::DataOut()
{
	// Check if sufficient space in command buffer
	const uint32_t newCharCnt = std::min(outBuffCount, maxCmdLen - 1 - buffPos);

	strncpy(&comCmd[buffPos], (char*)outBuff, newCharCnt);
	buffPos += newCharCnt;

	// Check if cr has been sent yet
	if (comCmd[buffPos - 1] == 13 || comCmd[buffPos - 1] == 10 || buffPos == maxCmdLen - 1) {
		comCmd[buffPos - 1] = '\0';
		cmdPending = true;
		buffPos = 0;
	}
}


void CDCHandler::ActivateEP()
{
	EndPointActivate(USB::CDC_In,   Direction::in,  EndPointType::Bulk);			// Activate CDC in endpoint
	EndPointActivate(USB::CDC_Out,  Direction::out, EndPointType::Bulk);			// Activate CDC out endpoint
	EndPointActivate(USB::CDC_Cmd,  Direction::in,  EndPointType::Interrupt);		// Activate Command IN EP

	EndPointTransfer(Direction::out, USB::CDC_Out, USB::ep_maxPacket);
}


void CDCHandler::ClassSetup(usbRequest& req)
{
	if (req.RequestType == DtoH_Class_Interface && req.Request == GetLineCoding) {
		SetupIn(req.Length, (uint8_t*)&lineCoding);
	}

	if (req.RequestType == HtoD_Class_Interface && req.Request == SetLineCoding) {
		// Prepare to receive line coding data in ClassSetupData
		usb->classPendingData = true;
		EndPointTransfer(Direction::out, 0, req.Length);
	}
}


void CDCHandler::ClassSetupData(usbRequest& req, const uint8_t* data)
{
	// ClassSetup passes instruction to set line coding - this is the data portion where the line coding is transferred
	if (req.RequestType == HtoD_Class_Interface && req.Request == SetLineCoding) {
		lineCoding = *(LineCoding*)data;
	}
}


int32_t CDCHandler::ParseInt(const std::string_view cmd, const char precedingChar, const int32_t low, const int32_t high) {
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


float CDCHandler::ParseFloat(const std::string_view cmd, const char precedingChar, const float low = 0.0f, const float high = 0.0f) {
	float val = -1.0f;
	const int8_t pos = cmd.find(precedingChar);		// locate position of character preceding
	if (pos >= 0 && std::strspn(&cmd[pos + 1], "0123456789.") > 0) {
		val = std::stof(&cmd[pos + 1]);
	}
	if (high > low && (val > high || val < low)) {
		printf("Must be a value between %f and %f\r\n", low, high);
		return low - 1.0f;
	}
	return val;
}


// Descriptor definition here as requires constants from USB class
const uint8_t CDCHandler::Descriptor[] = {
	// IAD Descriptor - Interface association descriptor for CDC class
	0x08,									// bLength (8 bytes)
	USB::IadDescriptor,						// bDescriptorType
	USB::CDCCmdInterface,					// bFirstInterface
	0x02,									// bInterfaceCount
	0x02,									// bFunctionClass (Communications and CDC Control)
	0x02,									// bFunctionSubClass
	0x01,									// bFunctionProtocol
	USB::CommunicationClass,				// String Descriptor

	// Interface Descriptor
	0x09,									// bLength: Interface Descriptor size
	USB::InterfaceDescriptor,				// bDescriptorType: Interface
	USB::CDCCmdInterface,					// bInterfaceNumber: Number of Interface
	0x00,									// bAlternateSetting: Alternate setting
	0x01,									// bNumEndpoints: 1 endpoint used
	0x02,									// bInterfaceClass: Communication Interface Class
	0x02,									// bInterfaceSubClass: Abstract Control Model
	0x01,									// bInterfaceProtocol: Common AT commands
	USB::CommunicationClass,				// iInterface

	// Header Functional Descriptor
	0x05,									// bLength: Endpoint Descriptor size
	USB::ClassSpecificInterfaceDescriptor,	// bDescriptorType: CS_INTERFACE
	0x00,									// bDescriptorSubtype: Header Func Desc
	0x10,									// bcdCDC: spec release number
	0x01,

	// Call Management Functional Descriptor
	0x05,									// bFunctionLength
	USB::ClassSpecificInterfaceDescriptor,	// bDescriptorType: CS_INTERFACE
	0x01,									// bDescriptorSubtype: Call Management Func Desc
	0x00,									// bmCapabilities: D0+D1
	0x01,									// bDataInterface: 1

	// ACM Functional Descriptor
	0x04,									// bFunctionLength
	USB::ClassSpecificInterfaceDescriptor,	// bDescriptorType: CS_INTERFACE
	0x02,									// bDescriptorSubtype: Abstract Control Management desc
	0x02,									// bmCapabilities

	// Union Functional Descriptor
	0x05,									// bFunctionLength
	USB::ClassSpecificInterfaceDescriptor,	// bDescriptorType: CS_INTERFACE
	0x06,									// bDescriptorSubtype: Union func desc
	0x00,									// bMasterInterface: Communication class interface
	0x01,									// bSlaveInterface0: Data Class Interface

	// Endpoint 2 Descriptor
	0x07,									// bLength: Endpoint Descriptor size
	USB::EndpointDescriptor,				// bDescriptorType: Endpoint
	USB::CDC_Cmd,							// bEndpointAddress
	USB::Interrupt,							// bmAttributes: Interrupt
	0x08,									// wMaxPacketSize
	0x00,
	0x10,									// bInterval

	//---------------------------------------------------------------------------

	// Data class interface descriptor
	0x09,									// bLength: Endpoint Descriptor size
	USB::InterfaceDescriptor,				// bDescriptorType:
	USB::CDCDataInterface,					// bInterfaceNumber: Number of Interface
	0x00,									// bAlternateSetting: Alternate setting
	0x02,									// bNumEndpoints: Two endpoints used
	0x0A,									// bInterfaceClass: CDC
	0x00,									// bInterfaceSubClass:
	0x00,									// bInterfaceProtocol:
	0x00,									// iInterface:

	// Endpoint OUT Descriptor
	0x07,									// bLength: Endpoint Descriptor size
	USB::EndpointDescriptor,				// bDescriptorType: Endpoint
	USB::CDC_Out,							// bEndpointAddress
	USB::Bulk,								// bmAttributes: Bulk
	LOBYTE(USB::ep_maxPacket),				// wMaxPacketSize:
	HIBYTE(USB::ep_maxPacket),
	0x00,									// bInterval: ignore for Bulk transfer

	// Endpoint IN Descriptor
	0x07,									// bLength: Endpoint Descriptor size
	USB::EndpointDescriptor,				// bDescriptorType: Endpoint
	USB::CDC_In,							// bEndpointAddress
	USB::Bulk,								// bmAttributes: Bulk
	LOBYTE(USB::ep_maxPacket),				// wMaxPacketSize:
	HIBYTE(USB::ep_maxPacket),
	0x00,									// bInterval: ignore for Bulk transfer
};


uint32_t CDCHandler::GetInterfaceDescriptor(const uint8_t** buffer) {
	*buffer = Descriptor;
	return sizeof(Descriptor);
}
