#pragma once
#include "USBClass.h"

class USBHost;


class MidiHostClass : public USBClass {
public:
	static constexpr uint8_t usbAudioClass = USBClass::Audio;
	static constexpr uint8_t usbMidiStreamingSubclass = 0x03;
	static constexpr const char* name = "MIDI";
	static constexpr uint8_t classCode = usbAudioClass;

	MidiHostClass(USBHost* usbHost, const char* name, uint8_t classCode) : USBClass(usbHost, name, classCode) {}
	HostStatus InterfaceInit() override;
	void InterfaceDeInit() override;
	HostStatus Process() override;
private:
	enum class MidiState { Init, Idle, SendData, Busy, GetData, Synch, Poll, Error };
	static constexpr uint8_t midiMinPoll = 1;

	uint8_t		midiBuffer[64];
	uint8_t		outPipe;
	uint8_t		inPipe;
	MidiState	state;
	uint8_t		outEp;
	uint8_t		inEp;
	uint16_t	packetSize;
	uint8_t		epAddr;
	uint16_t	poll;			// Not yet used - could be used to limit poll frequency
	uint32_t	timer;

	void MidiEvent(uint32_t* buff, uint16_t len);
};


extern MidiHostClass midiHostClass;
