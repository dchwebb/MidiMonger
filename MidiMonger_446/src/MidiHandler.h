#pragma once

#include "initialisation.h"
#include "USB.h"
#include "DACHandler.h"
#include "Config.h"
#include <list>
#include <vector>
#include <map>

class Config;
extern USB usb;
extern DACHandler dacHandler;
extern Config cfg;

#define MIDIQUEUESIZE 20

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
		uint8_t cfgChannelOrOutput : 4;	// holds channel of config data being transferred.	Byte position: ----dddd
		uint8_t configType : 4;			// holds type of config data being transferred.		Byte position: dddd----
		uint8_t configValue;			// value of note number or controller configured
	};
};

enum MIDIType {Unknown = 0, NoteOn = 0x9, NoteOff = 0x8, PolyPressure = 0xA, ControlChange = 0xB, ProgramChange = 0xC, ChannelPressure = 0xD, PitchBend = 0xE, System = 0xF };
enum class configSetting {type = 1, specificNote = 2, channel = 3, controller = 4};
enum class gateType {specificNote = 1, channelNote = 2, clock = 3};
enum class cvType {channelPitch = 1, controller = 2, pitchBend = 3, afterTouch = 4};

struct Gate {
	gateType type;
	uint8_t channel;
	uint8_t note;
	GPIO_TypeDef* gpioPort;
	uint8_t gpioPin;
	uint32_t gateOffTime;

	void gateInit()	{
		gpioPort->MODER |= (1 << (2 * gpioPin));			// Set to output
	}

	void gateOn() {
		gpioPort->BSRR |= (1 << gpioPin);					// Gate on
	}

	void gateOn(const uint32_t& offTime) {					// Gate on also setting off time for use with clocks
		gateOn();
		gateOffTime = offTime;
	}

	void gateOff() {
		gpioPort->BSRR |= (1 << (16 + gpioPin));			// Gate Off
	}

};


typedef std::list<uint8_t> activeNote;

struct channelNote {
	activeNote activeNotes;
	uint8_t voiceCount;
	float pitchbend;
};


class MidiHandler {
public:
	MidiHandler();
	void eventHandler(const uint32_t& data);
	void serialHandler(uint32_t data);
	void gateTimer();
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
			{gateType::channelNote,  1, 0,   GPIOA, 7},
			{gateType::channelNote,  2, 0,   GPIOA, 3},
			{gateType::channelNote,  3, 0,   GPIOA, 5},
			{gateType::channelNote,  4, 0,   GPIOC, 5},
			{gateType::specificNote, 10, 36, GPIOC, 1},
			{gateType::specificNote, 10, 38, GPIOB, 2},
			{gateType::specificNote, 10, 42, GPIOC, 9},
			{gateType::specificNote, 10, 46, GPIOC, 7}
	};

	struct CV {
		cvType type;
		uint8_t channel;
		DacAddress dacChannel;
		GPIO_TypeDef* gpioPort;
		uint8_t gpioPin;
		uint32_t offTime;
		uint8_t controller;
		uint8_t currentNote;
		uint8_t nextNote;
		void sendNote();
		void cvInit();
		void ledOn(float offMilliseconds);
	};

	CV cvOutputs[4] = {
			{cvType::channelPitch, 1, ChannelD, GPIOC, 14},
			{cvType::channelPitch, 2, ChannelC, GPIOC, 13},
			{cvType::channelPitch, 3, ChannelB, GPIOB, 8},
			{cvType::channelPitch, 4, ChannelA, GPIOB, 7}
	};

	uint8_t pitchBendSemiTones = 12;

private:

	void validateConfig(bool isGate, uint8_t num);
	void QueueInc();

	static channelNote channelNotes[16];			// For managing pitch bends and polyphony - stores voice count and active notes for each channel

	uint32_t ClockCount = 0;

	uint8_t Queue[MIDIQUEUESIZE];			// hold incoming serial MIDI bytes
	uint8_t QueueRead = 0;
	uint8_t QueueWrite = 0;
	uint8_t QueueSize = 0;

};



