#include "MidiHostClass.h"
#include "USBHost.h"

MidiHostClass midiHostClass(&usbHost, MidiHostClass::name, MidiHostClass::classCode);

HostStatus MidiHostClass::InterfaceInit()
{
	USBH_DbgLog("MIDI Class Init");

	auto interfaceDesc = usbHost->SelectInterface(classCode, usbMidiStreamingSubclass);
	if (interfaceDesc == nullptr) {
		USBH_DbgLog("Cannot Find the interface for %s class.", name);
		return HostStatus::Fail;
	}

	inPipe = 0;
	inEp = 0;
	outPipe = 0;
	outEp = 0;
	timer = 0;
	state = MidiState::Init;
	epAddr = interfaceDesc->epDesc[0].bEndpointAddress;
	packetSize = interfaceDesc->epDesc[0].wMaxPacketSize;
	poll = std::min(interfaceDesc->epDesc[0].bInterval, midiMinPoll);

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


void MidiHostClass::InterfaceDeInit()
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

	state = MidiState::Init;
}


HostStatus MidiHostClass::Process()
{
	// Manages state machine for Midi data transfers
	switch (state) {
	case MidiState::Init:
		if (usbHost->timer & 1) {		// Sync with start of Even Frame
			USBH_DbgLog("MIDI Class Process: Init");
			state = MidiState::GetData;
		}
		break;

	case MidiState::GetData:
		usbHost->StartTransfer(inPipe, USBHost::pidToken::Data, midiBuffer, packetSize);
		state = MidiState::Poll;
		timer = usbHost->timer;
		break;

	case MidiState::Poll:
		if (usbHost->GetURBState(inPipe) == USBHost::URBState::Done) {
			if (usbHost->GetTransferCount(inPipe) > 0) {
				MidiEvent(midiBuffer, packetSize);
			}
			state = MidiState::GetData;

		} else if (usbHost->GetURBState(inPipe) == USBHost::URBState::Stall) {
			if (usbHost->ClearFeature(epAddr) == HostStatus::OK) {		// Issue Clear Feature on interrupt IN endpoint
				state = MidiState::GetData;								// Change state to issue next IN token
			}
		}
		break;

	default:
		break;
	}

	return HostStatus::OK;
}


void MidiHostClass::MidiEvent(uint8_t* buff, uint16_t len)
{
	static uint32_t clkCnt = 0;

	uint32_t midiData = *(uint32_t*)buff;
	if (midiData == 0xF80F) {
		if (clkCnt == 0) {
			printf("Clock\r\n");
		}
		++clkCnt;
	} else {
		if (clkCnt > 0) {
			printf("Clock: %ld\r\n", clkCnt);
			clkCnt = 0;
		}
		printf("Midi: 0x%08lx\r\n", midiData);
	}
}


