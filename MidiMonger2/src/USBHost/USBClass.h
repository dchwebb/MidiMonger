#pragma once
#include "initialisation.h"

class USBHost;

enum class HostStatus {	OK, Busy, Fail, NotSupported, UnrecoveredError, ErrorSpeedUnknown };


// interface for USB class handlers
class USBClass {
public:
	USBHost* usbHost;
	USBClass(USBHost* usbHost, const char* name, uint8_t classCode);

	const char* name;
	uint8_t classCode;

	virtual HostStatus InterfaceInit() = 0;
	virtual void InterfaceDeInit() = 0;
	virtual HostStatus Process() = 0;

	enum MidiClassCodes : uint8_t {
		Audio = 0x01,
		CDC = 0x02,
		HID = 0x03,
		PID = 0x05,
		Image = 0x06,
		Printer = 0x07,
		MassStorage= 0x08,
		Hub = 0x09,
		CDCData = 0x0A,
		SmartCard = 0x0B,
		ContentSecurity = 0x0D,
		Video = 0x0E,
		PersonalHealthcare = 0x0F,
		AudioVideo = 0x10,
		Diagnostic = 0xDC,
		Miscellaneous = 0xFE
	};
};


