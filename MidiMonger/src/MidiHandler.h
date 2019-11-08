#pragma once

#include "initialisation.h"
#include "USB.h"
#include "DACHandler.h"
#include <list>
#include <vector>
#include <algorithm>

//class USB;
extern USB usb;
extern DACHandler dacHandler;

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
		uint8_t cfgChannelOrOutput : 4;		// holds channel of config data being transferred.	Byte position: ----dddd
		uint8_t configType : 4;			// holds type of config data being transferred.		Byte position: dddd----
		uint8_t configValue;			// value of note number or controller configured
	};
};

enum class configSetting { type = 1, specificNote = 2, channel = 3, controller = 4 };

enum class gateType {specificNote = 1, channelNote = 2, clock = 3};

struct Gate {
	gateType type;
	uint8_t channel;
	uint8_t note;
};

enum class cvType {channelPitch = 1, controller = 2, pitchBend = 3};

struct CV {
	cvType type;
	uint8_t channel;
	DacAddress dacChannel;
	uint8_t controller;
	uint8_t currentNote;
	uint8_t nextNote;
};

typedef std::list<uint8_t> activeNote;

struct channelNote {
	activeNote activeNotes;
	uint8_t voiceCount;
};




class MidiHandler {
public:
	MidiHandler();
	void eventHandler(uint32_t data);
	void setConfig();

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
			{cvType::channelPitch, 1, ChannelA},
			{cvType::channelPitch, 1, ChannelB},
			{cvType::channelPitch, 1, ChannelC},
			{cvType::channelPitch, 4, ChannelD}
	};
	std::list<uint8_t> midiNotes;		// list holds all MIDI notes currently sounding
	//std::vector< channelNote > channelNotes[16];
	channelNote channelNotes[16];
};


