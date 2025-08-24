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
	bool supportMiscClass;		// Set to true if implemented class supports the USB class code 0xEF 'Miscellaneous'

	virtual HostStatus InterfaceInit() = 0;
	virtual void InterfaceDeInit() = 0;
	virtual HostStatus Process() = 0;
};


