#pragma once
#include "InstrumentalTrack_Midi.h"
#include "InstrumentalTrack_DrumsLegacy.h"

template <size_t numPads, bool PRO_DRUMS>
struct Tracker<DrumNote<numPads, PRO_DRUMS>>
{
	struct Diff
	{
		bool flam = false;
		uint32_t notes[numPads + 1];
		Diff() { for (uint32_t& n : notes) n = UINT32_MAX; }
	} difficulties[5];

	bool enableDynamics = false;
	bool toms[3] = { !PRO_DRUMS, !PRO_DRUMS, !PRO_DRUMS };
};

template <>
struct Tracker<DrumNote_Legacy> : Tracker<DrumNote<5, true>> {};

template <>
void InstrumentTrackMidiParser<DrumNote<4, true>>::parseLaneColor(MidiNote note, const bool isON);

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::parseLaneColor(MidiNote note, const bool isON);

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseLaneColor(MidiNote note, const bool isON);

template <>
void InstrumentTrackMidiParser<DrumNote<4, true>>::toggleExtraValues(MidiNote note, const bool isON);

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::toggleExtraValues(MidiNote note, const bool isON);

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseText(std::string_view text);

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::init();

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::init();
