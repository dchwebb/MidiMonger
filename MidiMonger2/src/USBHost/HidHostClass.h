#pragma once
#include "USBClass.h"

class USBHost;


class HidHostClass : public USBClass {
public:
	static constexpr uint8_t usbHidClass = 0x03;
	static constexpr uint8_t usbBootSubclass = 0x01;
	static constexpr const char* name = "HID";
	static constexpr uint8_t classCode = usbHidClass;
	static constexpr uint8_t getReport = 0x01;

	HidHostClass(USBHost* usbHost, const char* name, uint8_t classCode) : USBClass(usbHost, name, classCode) {}
	HostStatus InterfaceInit() override;
	void InterfaceDeInit() override;
	HostStatus Process() override;
private:
	enum class HidState { Init, GetReport, Idle, SendData, Busy, GetData, Synch, Poll, Wait, Error };
	static constexpr uint8_t hidMinPoll = 1;

	uint8_t		hidBuffer[64];
	uint8_t		outPipe;
	uint8_t		inPipe;
	HidState	state;
	uint8_t		outEp;
	uint8_t		inEp;
	uint16_t	packetSize;
	uint8_t		epAddr;
	uint16_t	poll;			// Not yet used - could be used to limit poll frequency
	uint32_t	timer;

	bool GetReport(uint8_t reportType, uint8_t reportId);
	void HidEvent(uint32_t* buff, uint16_t len);
};


extern HidHostClass hidHostClass;
