#include "HidHostClass.h"
#include "USBHost.h"

HidHostClass hidHostClass(&usbHost, HidHostClass::name, HidHostClass::classCode);

HostStatus HidHostClass::InterfaceInit()
{
	USBH_DbgLog("HID Class Init");

	auto interfaceDesc = usbHost->SelectInterface(classCode, usbBootSubclass);
	if (interfaceDesc == nullptr) {
		USBH_DbgLog("Cannot Find the interface for %s class.", name);
		return HostStatus::Fail;
	}

	inPipe = 0;
	inEp = 0;
	outPipe = 0;
	outEp = 0;
	timer = 0;
	state = HidState::Init;
	epAddr = interfaceDesc->epDesc[0].bEndpointAddress;
	packetSize = interfaceDesc->epDesc[0].wMaxPacketSize;
	poll = std::min(interfaceDesc->epDesc[0].bInterval, hidMinPoll);

	uint8_t endpoints = std::min(interfaceDesc->bNumEndpoints, USBHost::maxNumEndpoints);

	// Decode endpoint IN and OUT address from interface descriptor
	for (uint8_t i = 0; i < endpoints; ++i) {
		if (interfaceDesc->epDesc[i].bEndpointAddress & 0x80) {
			inEp = interfaceDesc->epDesc[i].bEndpointAddress;
			inPipe = usbHost->AllocPipe(inEp);

			// Open pipe for IN endpoint
			usbHost->HostChannelInit(inPipe, inEp, usbHost->device.address, usbHost->device.speed, USBHost::Bulk, packetSize);
			usbHost->SetToggle(inPipe, 0);
		} else {
			outEp = interfaceDesc->epDesc[i].bEndpointAddress;
			outPipe  = usbHost->AllocPipe(outEp);

			// Open pipe for OUT endpoint
			usbHost->HostChannelInit(outPipe, outEp, usbHost->device.address, usbHost->device.speed, USBHost::Bulk, packetSize);
			usbHost->SetToggle(outPipe, 0);
		}
	}

	return HostStatus::OK;
}


void HidHostClass::InterfaceDeInit()
{
	USBH_DbgLog("MIDI Class DeInit");

	if (inPipe != 0) {
		usbHost->HaltChannel(inPipe);
		usbHost->FreePipe(inPipe);
		inPipe = 0;					// Reset the pipe as Free
	}

	if (outPipe != 0) {
		usbHost->HaltChannel(outPipe);
		usbHost->FreePipe(outPipe);
		outPipe = 0;    			// Reset the pipe as Free
	}

	state = HidState::Init;
}


HostStatus HidHostClass::Process()
{
	// Manages state machine for Hid data transfers
	switch (state) {
	case HidState::Init:
		if (usbHost->timer & 1) {			// Sync with start of Even Frame
			USBH_DbgLog("HID Class Process: Init");
			state = HidState::GetReportDesc;
		}
		break;

	case HidState::GetReportDesc:			// Get HID Report Descriptor
		if (GetReportDesc()) {
			hidDescriptor.PrintReportDesc();
			hidDescriptor.ParseReportDesc();
			state = HidState::GetData;
			USBH_DbgLog("HID Report Descriptor Parsed");
		}
		break;

	case HidState::GetData:
		usbHost->StartTransfer(inPipe, USBHost::pidToken::Data, hidBuffer, packetSize);
		state = HidState::Poll;
		timer = usbHost->timer;
		poll = SysTickVal;
		break;

	case HidState::Poll:
		if (usbHost->GetURBState(inPipe) == USBHost::URBState::Done) {
			if (usbHost->GetTransferCount(inPipe) > 0) {
				HidEvent(hidBuffer, packetSize);
			}
			state = HidState::Wait;

		} else if (usbHost->GetURBState(inPipe) == USBHost::URBState::Stall) {
			if (usbHost->ClearFeature(epAddr) == HostStatus::OK) {		// Issue Clear Feature on interrupt IN endpoint
				state = HidState::Wait;									// Change state to issue next IN token
			}
		}
		break;

	case HidState::Wait:
		state = HidState::GetData;
//		if (SysTickVal > poll + 1) {			// Test to apply a delay between each poll
//			state = HidState::GetData;
//		}

	default:
		break;
	}

	return HostStatus::OK;
}


bool HidHostClass::GetReportDesc()
{
	HostStatus status = usbHost->ClassRequest(USBHost::requestRecipientInterface | USBHost::requestTypeStandard,
			USBHost::RequestGetDescriptor,
			USB_DESC_HID_REPORT,
			0,
			hidDescriptor.rawDesc,
			hidDescriptor.hidDescSize);

	if (status == HostStatus::OK) {		// Commands successfully sent and Response Received
		return true;
	}
	return false;
}


bool HidHostClass::GetReport(uint8_t reportType, uint8_t reportId)
{
	// reportType = 1; reportId = 0
	HostStatus status = usbHost->ClassRequest(USBHost::requestRecipientInterface | USBHost::RequestTypeClass,
			getReport,
			(reportType << 8) | reportId,
			0,
			hidBuffer,
			packetSize);

	if (status == HostStatus::OK) {		// Commands successfully sent and Response Received
		return true;
	}
	return false;
}


uint32_t HidHostClass::ParseReport(uint8_t* buff, uint32_t offset, uint32_t size)
{
	// Parses HID report using bit offsets (which can cross byte boundaries)
	uint32_t firstByte = (offset >> 3);		// Get first byte
	uint32_t bitOffset = offset - (firstByte << 3);
	uint32_t data = buff[firstByte];
	if (bitOffset == 4) {
		data &= 0b1111;
		data = data << (size - bitOffset);
	} else {
		data = data << (size - 8);
	}

	uint32_t remainingBits = size - (8 - bitOffset);

	while (remainingBits) {
		++firstByte;
		if (remainingBits == 4) {
			data |= (buff[firstByte] >> 4);
			remainingBits = 0;
		} else {
			remainingBits -= 8;
			data |= (buff[firstByte] << remainingBits);
		}

	}
	return data;


}

void HidHostClass::HidEvent(uint8_t* buff, uint16_t len)
{
	// Moving mouse forwards = negative y
	int32_t mouse[hidDescriptor.ControlsCount] = {0};
	uint32_t buttons = 0;

	for (uint32_t i = 0; i < hidDescriptor.ControlsCount; ++i) {
		if (hidDescriptor.controls[i].Offset + hidDescriptor.controls[i].Size) {
			//mouse[i] = (hidDescriptor.controls[i].Size == 2) ? *(int16_t*)&buff[hidDescriptor.controls[i].Offset] : (int8_t)buff[hidDescriptor.controls[i].Offset];
			mouse[i] = ParseReport(buff, hidDescriptor.controls[i].Offset, hidDescriptor.controls[i].Size);
		}
	}

	if (hidDescriptor.buttonOffset + hidDescriptor.buttonCount) {
		buttons = buff[hidDescriptor.buttonOffset];
	}
	printf("Hid X: %ld Y: %ld; Wheel: %ld; Buttons: %lu\r\n", mouse[HidDescriptor::X], mouse[HidDescriptor::Y], mouse[HidDescriptor::Wheel], buttons);

}


