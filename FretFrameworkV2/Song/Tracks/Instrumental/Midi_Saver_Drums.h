#pragma once
#include "Midi_Saver_Instrument.h"
#include "Notes/DrumNote.h"
#include "Midi_Hold.h"

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<DrumPad_Pro, 4>>(MidiFileWriter& writer);

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<DrumPad, 5>>(MidiFileWriter& writer);

template <>
void Difficulty_Saver_Midi<DrumNote<DrumPad_Pro, 4>>::write_details(const DifficultyTrack<DrumNote<DrumPad_Pro, 4>>& track, const unsigned char index, MidiFileWriter& writer);

template <>
void Difficulty_Saver_Midi<DrumNote<DrumPad, 5>>::write_details(const DifficultyTrack<DrumNote<DrumPad, 5>>& track, const unsigned char index, MidiFileWriter& writer);
