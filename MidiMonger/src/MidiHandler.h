#pragma once

#include "initialisation.h"
#include "USB.h"
#include <list>
#include <algorithm>

//class USB;
extern USB usb;

union MidiData {
	MidiData(uint32_t d) : data(d) {};
	MidiData()  {};

	uint32_t data;
	struct {
		uint8_t CIN : 4;
		uint8_t cable : 4;
		uint8_t chn : 4;
		uint8_t msg : 4;
		uint8_t db1;
		uint8_t db2;
	};
};

enum class gateType {specificNote, channelNote, clock};

struct Gate {
	uint8_t channel;
	uint8_t note;
	gateType type;
};

enum class cvType {channelPitch, controller, pitchBend};

struct CV {
	uint8_t channel;
	uint8_t controller;
	cvType type;
};



class MidiHandler {
public:
	void eventHandler(MidiData event);
	void getMidiData(uint32_t data);

	Gate gateOutputs[8];
	CV cvOutputs[4];
	std::list<uint8_t> midiNotes;

};


