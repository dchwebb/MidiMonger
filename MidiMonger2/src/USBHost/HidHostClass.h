#pragma once
#include "USBClass.h"
#include "HidDescriptor.h"

class USBHost;

#define  USB_DESC_HID_REPORT                ((0x22 << 8) & 0xFF00U)

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
	enum class HidState { Init, GetReportDesc, Idle, SendData, Busy, GetData, Synch, Poll, Wait, Error };
	static constexpr uint8_t hidMinPoll = 1;

	static constexpr uint32_t hidBufferSize = 64;
	uint8_t		hidBuffer[hidBufferSize] __attribute__((__aligned__(8)));		// Aligned to 64 bits to make report parsing easier
	uint8_t		outPipe;
	uint8_t		inPipe;
	HidState	state;
	uint8_t		outEp;
	uint8_t		inEp;
	uint16_t	packetSize;
	uint8_t		epAddr;
	uint16_t	poll;			// Not yet used - could be used to limit poll frequency
	uint32_t	timer;

	HidDescriptor hidDescriptor;

	struct {
		uint16_t controls[3] = {0};
		uint32_t buttons = {0};

	} hidValues;

	int32_t mouseScale = 20;

	bool GetReportDesc();
	int32_t ParseReport(uint8_t* buff, uint32_t offset, uint32_t size);
	bool GetReport(uint8_t reportType, uint8_t reportId);
	void HidEvent(uint8_t* buff, uint16_t len);
};


extern HidHostClass hidHostClass;



