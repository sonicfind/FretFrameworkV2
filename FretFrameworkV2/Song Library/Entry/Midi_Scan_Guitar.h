#pragma once
#include "Midi_Scan.h"
#include "Notes/GuitarNote.h"

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner<GuitarNote<5>>::s_noteRange{ 59, 107 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner<GuitarNote<6>>::s_noteRange{ 58, 103 };

template <>
constexpr int Midi_Scanner<GuitarNote<5>>::s_defaultLanes[48] =
{
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

template <>
void Midi_Scanner<GuitarNote<5>>::parseText(std::string_view str);
