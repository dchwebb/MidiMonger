#pragma once
#include "USBClass.h"
#include "HidDescriptor.h"
#include "configManager.h"
#include "CommandHandler.h"

class USBHost;

#define  USB_DESC_HID_REPORT                ((0x22 << 8) & 0xFF00U)

class HidHostClass : public USBClass {
	friend CommandHandler;
public:
	static constexpr uint8_t usbHidClass = USBClass::HID;
	static constexpr uint8_t usbBootSubclass = 0x01;
	static constexpr const char* name = "HID";
	static constexpr uint8_t classCode = usbHidClass;
	static constexpr uint8_t getReport = 0x01;

	HidHostClass(USBHost* usbHost, const char* name, uint8_t classCode) : USBClass(usbHost, name, classCode) {}
	HostStatus InterfaceInit() override;
	void InterfaceDeInit() override;
	HostStatus Process() override;
	static void SetConfig();

	ConfigSaver configSaver = {
		.settingsAddress = &cfg,
		.settingsSize = sizeof(cfg),
		.validateSettings = SetConfig
	};

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

	enum CVSource {noCV = 0, mouseX, mouseY, mouseWheel};
	enum GateSource {noGate = 0, mouseBtn1, mouseBtn2, mouseBtn3, mouseBtn4, mouseBtn5, mouseBtn6, mouseBtn7, mouseBtn8};
	struct {
		CVSource cvSource[4];
		GateSource gateSource[8];
		int32_t mouseScale[3];
	} cfg = {mouseX, mouseY, mouseWheel, noCV,
			mouseBtn5, mouseBtn6, mouseBtn7, mouseBtn8,
			mouseBtn1, mouseBtn2, mouseBtn3, mouseBtn4,
			20, 20, 1000};

	bool GetReportDesc();
	int32_t ParseReport(uint8_t* buff, uint32_t offset, uint32_t size);
	bool GetReport(uint8_t reportType, uint8_t reportId);
	void HidEvent(uint8_t* buff, uint16_t len);
};


extern HidHostClass hidHostClass;



