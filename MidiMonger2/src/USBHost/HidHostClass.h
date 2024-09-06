#pragma once
#include "USBClass.h"

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

	static constexpr uint32_t hidDescSize = 256;

	static constexpr uint32_t hidBufferSize = 64;
	uint8_t		hidBuffer[hidBufferSize];
	uint8_t		outPipe;
	uint8_t		inPipe;
	HidState	state;
	uint8_t		outEp;
	uint8_t		inEp;
	uint16_t	packetSize;
	uint8_t		epAddr;
	uint16_t	poll;			// Not yet used - could be used to limit poll frequency
	uint32_t	timer;

	struct HIDDescriptor {
		uint8_t	 rawDesc[hidDescSize];		// Raw descriptor
		uint32_t currentOffset = 0;			// Caclulate report offsets on the fly

		uint32_t buttonOffset = 0;
		uint32_t buttonCount = 0;
		uint32_t xOffset = 0;
		uint32_t xSize = 0;
		uint32_t yOffset = 0;
		uint32_t ySize = 0;
		uint32_t wheelOffset = 0;
		uint32_t wheelSize = 0;

		/*
		 * 	0x05 0x01       Usage Page (Generic Desktop Controls)
	0x09 0x02       Usage (Mouse)
	0xA1 0x01       Collection (Application)
	0x09 0x01           Usage (Pointer)
	0xA1 0x00           Collection (Physical)
	0x05 0x09               Usage Page (Button)
	0x19 0x01               Usage Minimum (Button 1 (primary/trigger))
	0x29 0x03               Usage Maximum (Button 3 (tertiary))
	0x15 0x00               Logical Minimum (0x00)
	0x25 0x01               Logical Maximum (0x01)
	0x75 0x01               Report Size (0x01)
	0x95 0x03               Report Count (0x03)
	0x81 0x02               Input (Data,Variable,Absolute,No wrap,Linear,Preferred State,No Null position,Bit Field)
	0x75 0x05               Report Size (0x05)
	0x95 0x01               Report Count (0x01)
	0x81 0x01               Input (Constant,Array,Absolute,No wrap,Linear,Preferred State,No Null position,Bit Field)
	0x05 0x01               Usage Page (Generic Desktop Controls)
	0x09 0x30               Usage (X)
	0x09 0x31               Usage (Y)
	0x09 0x38               Usage (Wheel)
	0x15 0x81               Logical Minimum (0x81)
	0x25 0x7F               Logical Maximum (0x7F)
	0x75 0x08               Report Size (0x08)
	0x95 0x03               Report Count (0x03)
	0x81 0x06               Input (Data,Variable,Relative,No wrap,Linear,Preferred State,No Null position,Bit Field)
	0xC0                End Collection
	0x05 0xFF           Usage Page (Reserved 0xFF)
	0x09 0x02           Usage (Usage Page=Reserved 0xFF ID=0x02)
	0x15 0x00           Logical Minimum (0x00)
	0x25 0x01           Logical Maximum (0x01)
	0x75 0x01           Report Size (0x01)
	0x95 0x01           Report Count (0x01)
	0xB1 0x22           Feature (Data,Variable,Absolute,No wrap,Linear,No Preferred,No Null position,Non Volatile,Bit Field)
	0x75 0x07           Report Size (0x07)
	0x95 0x01           Report Count (0x01)
	0xB1 0x01           Feature (Constant,Array,Absolute,No wrap,Linear,Preferred State,No Null position,Non Volatile,Bit Field)
	0xC0            End Collection
		 */

		// FIXME - parsing doesn't account for items with 3 bytes; controls parsing assumes all report items are same size

		void ParseButtons(uint32_t& index) {
			uint32_t reportSize = 0;			// Size of report in bits
			buttonOffset = currentOffset;
			index += 2;
			while (index < hidDescSize && rawDesc[index] != 0x05 && rawDesc[index] != 0xC0) {		// Continue until end of descriptor or next Usage Page
				index += 2;
				if (rawDesc[index] == 0x29) {		// Usage Maximum gives number of buttons
					buttonCount = rawDesc[index + 1];
				}
				if ((rawDesc[index] == 0x75 && rawDesc[index + 2] == 0x95) || (rawDesc[index] == 0x95 && rawDesc[index + 2] == 0x75)) {		// Report Size and Report Count
					reportSize += rawDesc[index + 1] * rawDesc[index + 3];
				}
			}
			currentOffset += (reportSize >> 3);
		}

		void ParseControls(uint32_t& index)
		{
			// Get size and count of report
			uint32_t reportSize = 0;			// Size of report in bits
			uint32_t reportCount = 0;			// Number of reports
			index += 2;
			uint32_t i = index;
			while (i < hidDescSize && rawDesc[i] != 0x05 && rawDesc[i] != 0xC0 && (reportSize == 0 || reportCount == 0)) {
				if (rawDesc[i] == 0x75) {
					reportSize = rawDesc[i + 1];
				}
				if (rawDesc[i] == 0x95) {
					reportCount = rawDesc[i + 1];
				}
				i += 2;
			}

			while (index < hidDescSize && rawDesc[index] != 0x05 && rawDesc[index] != 0xC0) {		// Continue until end of descriptor or next Usage Page
				if (rawDesc[index] == 0x09) {			// Usage
					if (rawDesc[index + 1] == 0x30) {		// Usage (X)
						xOffset = currentOffset;
						xSize = (reportSize >> 3);
					}
					if (rawDesc[index + 1] == 0x31) {		// Usage (Y)
						yOffset = currentOffset;
						ySize = (reportSize >> 3);
					}
					if (rawDesc[index + 1] == 0x38) {		// Usage (Wheel)
						wheelOffset = currentOffset;
						wheelSize = (reportSize >> 3);
					}
					currentOffset += (reportSize >> 3);
				}
				index += 2;
			}
		}
	} hidDescriptor;

	bool GetReportDesc();
	void ParseReportDesc();
	bool GetReport(uint8_t reportType, uint8_t reportId);
	void HidEvent(uint8_t* buff, uint16_t len);
};


extern HidHostClass hidHostClass;
