#pragma once
#include "DrumScan.h"
#include "InstrumentalScan_Midi.h"

template<size_t numPads, bool PRO_DRUMS>
struct Midi_Scanner_Extensions<DrumNote<numPads, PRO_DRUMS>>
{
	bool expertPlus = false;
	bool doubleBass = false;
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner<DrumNote<5, false>>::s_noteRange{ 60, 102 };

namespace DrumMidi
{
	template <bool NoteOn, class T>
	unsigned char ParseLaneColor(Midi_Scanner<T>& scanner, MidiNote note)
	{
		if (note.value == 95)
		{
			if (scanner.m_ext.expertPlus)
				return 0;

			if constexpr (!NoteOn)
			{
				if (scanner.m_ext.doubleBass)
				{
					scanner.m_ext.expertPlus = true;
					return 16;
				}
			}
			scanner.m_ext.doubleBass = true;
		}
		else
		{
			const int noteValue = note.value - Midi_Scanner<T>::s_noteRange.first;
			const int diff = Midi_Scanner<T>::s_diffValues[noteValue];
			if (scanner.m_difficulties[diff].active)
				return 0;

			const int lane = scanner.m_laneValues[noteValue];
			if (lane < T::GetLaneCount())
			{
				if constexpr (!NoteOn)
				{
					if (scanner.m_difficulties[diff].notes[lane])
					{
						scanner.m_difficulties[diff].active = true;
						return 1 << diff;
					}
				}
				scanner.m_difficulties[diff].notes[lane] = true;
			}
		}
		return 0;
	}
}


template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote<4, true>>::parseLaneColor(MidiNote note)
{
	m_values.m_subTracks |= DrumMidi::ParseLaneColor<NoteOn>(*this, note);
	return values.m_subTracks == 31;
}

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote<5, false>>::parseLaneColor(MidiNote note)
{
	m_values.m_subTracks |= DrumMidi::ParseLaneColor<NoteOn>(*this, note);
	return values.m_subTracks == 31;
}
