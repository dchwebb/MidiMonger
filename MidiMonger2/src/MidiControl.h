#pragma once

#include "initialisation.h"
#include "DACHandler.h"
#include "CDCHandler.h"
#include "configManager.h"
#include <list>


class MidiControl {
	friend class CommandHandler;
public:
	enum MIDIType { Unknown = 0, NoteOn = 0x9, NoteOff = 0x8, PolyPressure = 0xA, ControlChange = 0xB,
		ProgramChange = 0xC, ChannelPressure = 0xD, PitchBend = 0xE, System = 0xF };
	enum class GateType { specificNote = 1, channelNote = 2, clock = 3 };
	enum class CvType { channelPitch = 1, controller = 2, pitchBend = 3, afterTouch = 4 };
	enum class ConfigSetting { type = 1, specificNote = 2, channel = 3, controller = 4 };
	enum class OutputType { gate, cv };
	enum class LightShowType {startup, connection};
	MidiControl();
	void MidiEvent(const uint32_t data);
	void SerialHandler(uint32_t data);
	static void SetConfig();
	void GateTimer();
	void CalcPortamento();
	void LightShow(LightShowType);

	// Commands allowing external classes (eg USB HID) to output data
	void SendCV(uint16_t dacOutput, uint8_t channel, uint32_t ledTimoutMS);
	void SendGate(uint8_t gate, bool on);

	ConfigSaver configSaver = {
		.settingsAddress = &cfg,
		.settingsSize = sizeof(cfg),
		.validateSettings = SetConfig
	};


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

	static constexpr uint32_t midiDebugSize = 400;
	MidiData debugEvents[midiDebugSize];
	uint32_t midiDebugCount = 0;

private:
	void GatesOff();
	void QueueInc();
	void MatchChannelSetting(OutputType isGate, uint8_t num);

	typedef std::list<uint8_t> activeNote;

	struct ChannelNote {
		activeNote activeNotes;
		uint8_t voiceCount;
		uint8_t lastVoice;					// for round robin assignment of notes to voices
		float pitchbend;
	};
	ChannelNote channelNotes[16];			// For managing pitch bends and polyphony - stores voice count and active notes for each channel

	struct Gate {
		GateType type;
		uint8_t channel;
		uint8_t note;
		GpioPin output;
		uint32_t gateOffTime;
		Gate(GateType type, uint8_t channel, uint8_t note, GPIO_TypeDef* gpioPort, uint8_t gpioPin) :
			type(type), channel(channel), note(note), output(gpioPort, gpioPin, GpioPin::Type::Output) {};

		void GateOn(const uint32_t offTime) {					// Gate on also setting off time for use with clocks
			output.SetHigh();
			gateOffTime = offTime;
		}
	};

#ifdef V1_HARDWARE
	Gate gateOutputs[8] = {
			{GateType::channelNote,  1, 0,   GPIOA, 7},
			{GateType::channelNote,  2, 0,   GPIOA, 3},
			{GateType::channelNote,  3, 0,   GPIOA, 5},
			{GateType::channelNote,  4, 0,   GPIOC, 5},
			{GateType::specificNote, 10, 36, GPIOC, 1},
			{GateType::specificNote, 10, 38, GPIOB, 2},
			{GateType::specificNote, 10, 42, GPIOC, 9},
			{GateType::specificNote, 10, 46, GPIOC, 7}
	};
#else
	Gate gateOutputs[8] = {
			{GateType::channelNote,  1, 0,   GPIOB, 0},
			{GateType::channelNote,  2, 0,   GPIOC, 4},
			{GateType::channelNote,  3, 0,   GPIOB, 2},
			{GateType::channelNote,  4, 0,   GPIOA, 7},
			{GateType::specificNote, 10, 36, GPIOB, 12},
			{GateType::specificNote, 10, 38, GPIOA, 5},
			{GateType::specificNote, 10, 42, GPIOB, 14},
			{GateType::specificNote, 10, 46, GPIOC, 3}
	};
#endif

	struct CV {
		CvType type;
		uint8_t channel;
		DACHandler::Address dacChannel;
		GpioPin led;
		uint32_t offTime;
		uint8_t controller;
		uint8_t currentNote;
		uint8_t prevNote;
		uint8_t nextNote;

		// Portamento settings
		bool portamento;
		float currentOutput;
		float targetOutput;

		CV(CvType type,	uint8_t channel, DACHandler::Address dacChannel, GPIO_TypeDef* gpioPort, uint8_t gpioPin) :
			type(type), channel(channel), dacChannel(dacChannel), led(gpioPort, gpioPin, GpioPin::Type::Output) {};
		void SendNote();
		void LedOn(float offMs) {
			led.SetHigh();
			offTime = SysTickVal + (offMs * 2.5);				// pass gate off time: each tick is around 400us: 2500 x 400us = 1 second
		}
	} cvOutputs[4] = {
			{CvType::channelPitch, 1, DACHandler::ChannelD, GPIOC, 14},
			{CvType::channelPitch, 2, DACHandler::ChannelC, GPIOC, 13},
			{CvType::channelPitch, 3, DACHandler::ChannelB, GPIOB, 8},
			{CvType::channelPitch, 4, DACHandler::ChannelA, GPIOB, 7}
	};

	static constexpr float dacOffsetDefault = 25.3f;
	static constexpr float dacScaleDefault = 74.0f;
	static constexpr float pitchBendSemiTonesDefault = 12.0f;						// Number of semitones for a full pitchbend
	struct Config {
		struct GateConfig {
			GateType type;
			uint8_t channel;
			uint8_t note;
		} gates[8];

		struct CVConfig {
			CvType type;
			uint8_t channel;
			uint8_t controller;
		} cvs[4];

		float pitchBendSemiTones = pitchBendSemiTonesDefault;						// Number of semitones for a full pitchbend
		uint8_t portamento = 0;

		float dacOffset = dacOffsetDefault;
		float dacScale = dacScaleDefault;
	} cfg;


	float dacScaleCalc;							// dac scaling adjusted for 16 bit output
	float portamentoCalc;						// Calculated speed of portamento effect

	static constexpr uint32_t QueueSize = 50;
	uint32_t ClockCount = 0;

	uint8_t queue[QueueSize];			// hold incoming serial MIDI bytes
	uint8_t queueRead = 0;
	uint8_t queueWrite = 0;
	uint8_t queueCount = 0;
};

extern MidiControl midiControl;


/*	Configure gates to default values for GM drum sounds
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
