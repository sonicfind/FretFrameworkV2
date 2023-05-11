#pragma once
#include "Midi_Saver_Instrument.h"
#include "Notes/DrumNote.h"
#include "Midi_Hold.h"

template <>
SimpleFlatMap<std::vector<std::u32string>> Midi_Saver_Instrument::GetStartingEvent<DrumNote<DrumPad_Pro, 4>>() noexcept;

template <>
SimpleFlatMap<std::vector<std::u32string>> Midi_Saver_Instrument::GetStartingEvent<DrumNote<DrumPad, 5>>() noexcept;

template <>
bool Difficulty_Saver_Midi::Get_Details<DrumNote<DrumPad_Pro, 4>>(const DifficultyTrack<DrumNote<DrumPad_Pro, 4>>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff);

template <>
bool Difficulty_Saver_Midi::Get_Details<DrumNote<DrumPad, 5>>(const DifficultyTrack<DrumNote<DrumPad, 5>>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff);
