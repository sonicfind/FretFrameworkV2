#pragma once
#include "InstrumentalScan.h"
#include "Notes/DrumNote.h"

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalScan<DrumNote<5, false>>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
void InstrumentalScan<DrumNote<4, true>>::parseLaneColor(Midi_Scanner& scanner, MidiNote note)
{
	if (note.value == 95)
	{
		if constexpr (!NoteOn)
		{
			if (scanner.difficulties[3].notes[0])
			{
				m_subTracks |= 8;
				scanner.difficulties[3].active = true;
			}
		}
		scanner.difficulties[3].notes[0] = true;
	}
	else
	{
		const int noteValue = note.value - s_noteRange.first;
		const int diff = s_diffValues[noteValue];
		if (!scanner.difficulties[diff].active)
		{
			const int lane = scanner.laneValues[noteValue];
			if (lane < 5)
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
}

template <>
template <bool NoteOn>
void InstrumentalScan<DrumNote<5, false>>::parseLaneColor(Midi_Scanner& scanner, MidiNote note)
{
	if (note.value == 95)
	{
		if constexpr (!NoteOn)
		{
			if (scanner.difficulties[3].notes[0])
			{
				m_subTracks |= 8;
				scanner.difficulties[3].active = true;
			}
		}
		scanner.difficulties[3].notes[0] = true;
	}
	else
	{
		const int noteValue = note.value - s_noteRange.first;
		const int diff = s_diffValues[noteValue];
		if (!scanner.difficulties[diff].active)
		{
			const int lane = scanner.laneValues[noteValue];
			if (lane < 6)
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
}
