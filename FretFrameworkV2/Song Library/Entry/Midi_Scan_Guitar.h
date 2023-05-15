#pragma once
#include "Midi_Scan_Instrument.h"
#include "Notes/GuitarNote.h"

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner<GuitarNote<5>>::s_noteRange{ 59, 107 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner<GuitarNote<6>>::s_noteRange{ 58, 103 };

template <>
Midi_Scanner_Instrument::Scanner_Lanes<GuitarNote<5>>::Scanner_Lanes();

template <>
void Midi_Scanner_Instrument::Scanner<GuitarNote<5>>::parseText(std::string_view str);
