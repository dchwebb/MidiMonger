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
	// Used primarily for transferring configuration data to the editor
	struct {
		uint8_t Code;
		uint8_t db0;
		uint8_t configChannel : 4;		// holds channel of config data being transferred
		uint8_t configType : 4;			// holds type of config data being transferred
		uint8_t configValue;			// value of note number or controller configured
	};
};

enum class gateType {specificNote, channelNote, clock};

struct Gate {
	gateType type;
	uint8_t channel;
	uint8_t note;
};

enum class cvType {channelPitch, controller, pitchBend};

struct CV {
	cvType type;
	uint8_t channel;
	uint8_t controller;
};



class MidiHandler {
public:
	MidiHandler();
	void eventHandler(uint32_t data);

	// configure gates to default values for GM drum sounds
	/*
	36 Bass Drum 1
	37 Rim Shot (Side Stick)
	38 Acoustic Snare
	39 Hand Clap
	40 Electric Snare
	41 Low Tom A
	42 Closed Hi-Hat
	43 Low Tom B
	44 Pedal Hi-Hat
	45 Mid Tom A
	46 Open Hi-Hat
	47 Mid Tom B
	48 High Tom A
	49 Crash Cymbal
	*/
	Gate gateOutputs[8] = {
			{gateType::specificNote, 10, 36},
			{gateType::specificNote, 10, 38},
			{gateType::specificNote, 10, 42},
			{gateType::specificNote, 10, 46},
			{gateType::channelNote, 1},
			{gateType::channelNote, 2},
			{gateType::channelNote, 3},
			{gateType::channelNote, 4}
	};

	CV cvOutputs[4] = {
			{cvType::channelPitch, 1},
			{cvType::channelPitch, 2},
			{cvType::channelPitch, 3},
			{cvType::channelPitch, 4}
	};
	std::list<uint8_t> midiNotes;		// list holds all MIDI notes currently sounding

};


