#pragma once
enum class MidiEventType : unsigned char
{
	Sequence_Number = 0x00,
	Text = 0x01,
	Text_Copyright = 0x02,
	Text_TrackName = 0x03,
	Text_InstrumentName = 0x04,
	Text_Lyric = 0x05,
	Text_Marker = 0x06,
	Text_CuePoint = 0x07,
	Text_EnumLimit = 0x0F,
	MIDI_Channel_Prefix = 0x20,
	End_Of_Track = 0x2F,
	Tempo = 0x51,
	SMPTE_Offset = 0x54,
	Time_Sig = 0x58,
	Key_Sig = 0x59,
	Sequencer_Specific_Meta_Event = 0x7F,

	Note_Off = 0x80,
	Note_On = 0x90,
	Key_Pressure = 0xA0,
	Control_Change = 0xB0,
	Program_Change = 0xC0,
	Channel_Pressure = 0xD0,
	Pitch_Wheel = 0xE0,

	SysEx = 0xF0,
	Undefined = 0xF1,
	Song_Position = 0xF2,
	Song_Select = 0xF3,
	Undefined_2 = 0xF4,
	Undefined_3 = 0xF5,
	Tune_Request = 0xF6,
	SysEx_End = 0xF7,
	Timing_Clock = 0xF8,
	Undefined_4 = 0xF9,
	Start_Sequence = 0xFA,
	Continue_Sequence = 0xFB,
	Stop_Sequence = 0xFC,
	Undefined_5 = 0xFD,
	Active_Sensing = 0xFE,
	Reset_Or_Meta = 0xFF,
};

struct MidiNote
{
	unsigned char value;
	unsigned char velocity;
};

struct ControlChange
{
	unsigned char controller = 0;
	unsigned char value = 0;
};

struct MidiEvent
{
	uint64_t position = 0;
	MidiEventType type = MidiEventType::Reset_Or_Meta;
	unsigned char channel = 0;
};
