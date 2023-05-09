#pragma once
#include "Midi_Saver_Instrument.h"
#include "Notes/DrumNote.h"
#include "Midi_Hold.h"

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<4, true>>(MidiFileWriter& writer);

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<5, false>>(MidiFileWriter& writer);

template <>
void Difficulty_Saver_Midi<DrumNote<4, true>>::write_details(const DifficultyTrack<DrumNote<4, true>>& track, const unsigned char index, MidiFileWriter& writer);

template <>
void Difficulty_Saver_Midi<DrumNote<5, false>>::write_details(const DifficultyTrack<DrumNote<5, false>>& track, const unsigned char index, MidiFileWriter& writer);
