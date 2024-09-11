#include <stdio.h>

struct HidDescriptor
{
	enum ByteCodes : uint8_t {StartCollection = 0xA1, EndCollection = 0xC0,
		UsagePage = 0x05, UsageMaximum = 0x29, UsageX = 0x30, UsageY = 0x31, UsageWheel = 0x38,
		ReportSize = 0x75, ReportCount = 0x95,
		LogicalMinimum_8 = 0x15, LogicalMinimum_16 = 0x16,
		LogicalMaximum_8 = 0x25, LogicalMaximum_16 = 0x26};
	enum WordCodes : uint16_t {CollectionPhysical = 0x00A1, UsagePageButton = 0x0905, UsagePageDesktopControls = 0x0105};
	enum Controls {X, Y, Wheel, ControlsCount};

	static constexpr uint32_t hidDescSize = 256;

	uint8_t	 rawDesc[hidDescSize];		// Raw descriptor

	uint32_t buttonOffset = 0;
	uint32_t buttonCount = 0;
	struct Control {
		uint32_t Offset = 0;
		uint32_t Size = 0;
	} controls[3];

	// Parsing variables
	uint32_t parsePos = 0;				// Parser position in raw descriptor
	uint32_t currentOffset = 0;			// Caclulate report offsets on the fly

	bool IncPos()
	{
		// increments position in raw descriptor from item to item returning false if at end of collection
		if (rawDesc[parsePos] == EndCollection) {										// End of collection
			parsePos += 1;
		} else if (rawDesc[parsePos] == LogicalMinimum_16 || rawDesc[parsePos] == LogicalMaximum_16) {	// 16 bit logical min/max
			parsePos += 3;
		} else {
			parsePos += 2;
		}
		return (parsePos < hidDescSize && rawDesc[parsePos] != EndCollection);		// Not at end of descriptor and not at end of collection
	}


	void ParseReportDesc()
	{
		parsePos = 0;
		currentOffset = 0;

		while (IncPos()) {

			// FIXME Some descriptors have a byte of constant data at the beginning?? hack to get correct offset
			if (*(uint16_t*)&rawDesc[parsePos] == UsagePageDesktopControls) {
				currentOffset += 8;
			}

			if (*(uint16_t*)&rawDesc[parsePos] == CollectionPhysical) {					// Collection (Physical)
				while (IncPos()) {

					if (*(uint16_t*)&rawDesc[parsePos] == UsagePageButton) {			// Usage Page (Button)
						ParseButtons();
					}
					if (*(uint16_t*)&rawDesc[parsePos] == UsagePageDesktopControls) {	// Usage Page (Generic Desktop Controls)
						ParseControls();
					}
				}
			}
		}
	}


	void ParseButtons()
	{
		uint32_t reportSize = 0;			// Size of report in bits
		buttonOffset = currentOffset;

		while (IncPos() && rawDesc[parsePos] != UsagePage) {	// Continue until end of descriptor or next Usage Page
			if (rawDesc[parsePos] == UsageMaximum) {			// Usage Maximum gives number of buttons
				buttonCount = rawDesc[parsePos + 1];
			}
			if ((rawDesc[parsePos] == ReportSize && rawDesc[parsePos + 2] == ReportCount) ||
					(rawDesc[parsePos] == ReportCount && rawDesc[parsePos + 2] == ReportSize)) {
				reportSize += rawDesc[parsePos + 1] * rawDesc[parsePos + 3];
			}
		}
		currentOffset += reportSize;
	}


	uint32_t GetReportSize()
	{
		// Get size and count (unused) of report
		uint32_t reportSize = 0;			// Size of report in bits
		uint32_t reportCount = 0;			// Number of reports
		uint32_t oldParsePos = parsePos;
		while (IncPos() && rawDesc[parsePos] != UsagePage && (reportSize == 0 || reportCount == 0)) {
			if (rawDesc[parsePos] == ReportSize) {
				reportSize = rawDesc[parsePos + 1];
			}
			if (rawDesc[parsePos] == ReportCount) {
				reportCount = rawDesc[parsePos + 1];
			}
		}
		parsePos = oldParsePos;				// Reset parsePos now report size is known
		return reportSize;
	}


	void ParseControls()
	{
		while (IncPos() && rawDesc[parsePos] != UsagePage) {	// Continue until end of descriptor or next Usage Page
			if (rawDesc[parsePos] == 0x09) {					// Usage
				uint32_t reportSize = GetReportSize();
				if (rawDesc[parsePos + 1] == UsageX) {			// Usage (X)
					controls[X].Offset = currentOffset;
					controls[X].Size = reportSize;
				}
				if (rawDesc[parsePos + 1] == UsageY) {			// Usage (Y)
					controls[Y].Offset = currentOffset;
					controls[Y].Size = reportSize;
				}
				if (rawDesc[parsePos + 1] == UsageWheel) {		// Usage (Wheel)
					controls[Wheel].Offset = currentOffset;
					controls[Wheel].Size = reportSize;
				}
				currentOffset += reportSize;
			}
		}
	}


	void PrintReportDesc()
	{
		// print HID report descriptor
		printf("Hid Report Descriptor: \r\n");
		uint32_t collections = 0;			// End printing when number of end collections matches number of collections
		for (uint32_t i = 0; i < hidDescSize; ++i) {
			printf("%02X ", rawDesc[i]);

			if (rawDesc[i] == StartCollection) {
				++collections;
			}
			if (rawDesc[i] == EndCollection) {
				--collections;
				if (collections == 0) {
					break;
				}
			}
		}
		printf("\r\n");

		// Print parsed offsets
		printf("Buttons - Offset: %ld Count %ld\r\n"
			   "Mouse X - Offset: %ld Size %ld\r\n"
			   "Mouse Y - Offset: %ld Size %ld\r\n"
			   "Mouse Wheel - Offset: %ld Size %ld\r\n ",
			   buttonOffset, buttonCount,
			   controls[X].Offset, controls[X].Size,
			   controls[Y].Offset, controls[Y].Size,
			   controls[Wheel].Offset, controls[Wheel].Size);

	}
};



/*
	0x05 0x01       Usage Page (Generic Desktop Controls)
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



	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x02,        // Usage (Mouse)
	0xA1, 0x01,        // Collection (Application)
	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x09, 0x02,        //   Usage (Mouse)
	0xA1, 0x02,        //   Collection (Logical)
	0x85, 0x1A,        //     Report ID (26)
	0x09, 0x01,        //     Usage (Pointer)
	0xA1, 0x00,        //     Collection (Physical)
	0x05, 0x09,        //       Usage Page (Button)
	0x19, 0x01,        //       Usage Minimum (0x01)
	0x29, 0x05,        //       Usage Maximum (0x05)
	0x95, 0x05,        //       Report Count (5)
	0x75, 0x01,        //       Report Size (1)
	0x15, 0x00,        //       Logical Minimum (0)
	0x25, 0x01,        //       Logical Maximum (1)
	0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x03,        //       Report Size (3)
	0x95, 0x01,        //       Report Count (1)
	0x81, 0x01,        //       Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x01,        //       Usage Page (Generic Desktop Ctrls)
	0x09, 0x30,        //       Usage (X)
	0x09, 0x31,        //       Usage (Y)
	0x95, 0x02,        //       Report Count (2)
	0x75, 0x10,        //       Report Size (16)
	0x16, 0x01, 0x80,  //       Logical Minimum (-32767)
	0x26, 0xFF, 0x7F,  //       Logical Maximum (32767)
	0x81, 0x06,        //       Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0x09, 0x38,        //       Usage (Wheel)
	0x35, 0x00,        //       Physical Minimum (0)
	0x45, 0x00,        //       Physical Maximum (0)
	0x95, 0x01,        //       Report Count (1)
	0x75, 0x10,        //       Report Size (16)
	0x16, 0x01, 0x80,  //       Logical Minimum (-32767)
	0x26, 0xFF, 0x7F,  //       Logical Maximum (32767)
	0x81, 0x06,        //       Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //     End Collection
	0xC0,              //   End Collection
	0xC0,              // End Collection
	0x8C,              // Unknown (bTag: 0x08, bType: 0x03)
 */
