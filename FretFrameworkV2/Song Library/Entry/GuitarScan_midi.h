#pragma once
#include "InstrumentalScan.h"
#include "Notes/GuitarNote.h"

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalScan<GuitarNote<5>>::s_noteRange{ 59, 107 };

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalScan<GuitarNote<6>>::s_noteRange{ 58, 103 };

template <>
constexpr int InstrumentalScan<GuitarNote<5>>::Midi_Scanner::s_defaultLanes[48] =
{
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

template <>
template <bool NoteOn>
void InstrumentalScan<GuitarNote<5>>::parseLaneColor(Midi_Scanner& scanner, MidiNote note)
{
	const int noteValue = note.value - s_noteRange.first;
	const int diff = s_diffValues[noteValue];

	if (!scanner.difficulties[diff].active)
	{
		const int lane = scanner.laneValues[noteValue];
		if (0 <= lane && lane < 6)
		{
			if constexpr (!NoteOn)
			{
				if (scanner.difficulties[diff].notes[lane])
				{
					m_subTracks |= 1 << diff;
					scanner.difficulties[diff].active = true;
				}
			}
			scanner.difficulties[diff].notes[lane] = true;
		}
	}
}

template <>
void InstrumentalScan<GuitarNote<5>>::parseText(Midi_Scanner& scanner, std::string_view str);
