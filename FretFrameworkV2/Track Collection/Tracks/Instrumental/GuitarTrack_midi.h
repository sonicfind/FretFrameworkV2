#pragma once
#include "InstrumentalTrack_Midi.h"
#include "Notes/GuitarNote.h"

template <>
struct Tracker<GuitarNote<5>>
{
	using ValCombo = std::pair<SpecialPhraseType, uint32_t>;
	struct
	{
		bool sliderNotes = false;
		bool hopoOn = false;
		bool hopoOff = false;
		ValCombo starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
		ValCombo faceOff[2] = { { SpecialPhraseType::FaceOff_Player1, UINT32_MAX } , { SpecialPhraseType::FaceOff_Player2, UINT32_MAX } };
		uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	} difficulties[5];
};

template <>
struct Tracker<GuitarNote<6>>
{
	struct
	{
		bool sliderNotes = false;
		bool hopoOn = false;
		bool hopoOff = false;
		uint32_t notes[7] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	} difficulties[5];
};

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseLaneColor(MidiNote note);

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::parseLaneColor(MidiNote note);

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::init();

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseSysEx(std::string_view sysex);

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseText(std::string_view text);

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::init();

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::parseSysEx(std::string_view sysex);
