#pragma once
#include "InstrumentalScan_Midi.h"
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

template<>
template <bool NoteOn>
bool Midi_Scanner<GuitarNote<5>>::parseLaneColor(ScanValues& values, MidiNote note)
{
	const int noteValue = note.value - s_noteRange.first;
	const int diff = s_diffValues[noteValue];

	if (m_difficulties[diff].active)
		return false;

	const int lane = m_laneValues[noteValue];
	if (lane < 6)
	{
		if constexpr (!NoteOn)
		{
			if (m_difficulties[diff].notes[lane])
			{
				values.addSubTrack(diff);
				m_difficulties[diff].active = true;
				return values.m_subTracks == 15;
			}
		}
		m_difficulties[diff].notes[lane] = true;
	}
	return false;
}

template <>
void Midi_Scanner<GuitarNote<5>>::parseText(std::string_view str);
