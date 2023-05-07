#pragma once
#include "DrumScan.h"
#include "InstrumentalScan_Midi.h"

template<>
struct InstrumentalScan_Midi::Midi_Scanner_Extensions<DrumNote<5, false>>
{
	bool expertPlus = false;
	bool doubleBass = false;
};

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalScan_Midi::Midi_Scanner<DrumNote<5, false>>::s_noteRange{ 60, 102 };

template<>
struct InstrumentalScan_Midi::Midi_Scanner_Extensions<DrumNote<4, true>>
{
	bool expertPlus = false;
	bool doubleBass = false;
};

namespace DrumMidi
{
	template <bool NoteOn, class T>
	unsigned char ParseLaneColor(InstrumentalScan_Midi::Midi_Scanner<T>& scanner, MidiNote note)
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
			const int noteValue = note.value - InstrumentalScan_Midi::Midi_Scanner<T>::s_noteRange.first;
			const int diff = InstrumentalScan_Midi::Midi_Scanner<T>::s_diffValues[noteValue];
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
bool InstrumentalScan_Midi::Midi_Scanner<DrumNote<4, true>>::parseLaneColor(ScanValues& values, MidiNote note)
{
	values.m_subTracks |= DrumMidi::ParseLaneColor<NoteOn>(*this, note);
	return values.m_subTracks == 31;
}

template <>
template <bool NoteOn>
bool InstrumentalScan_Midi::Midi_Scanner<DrumNote<5, false>>::parseLaneColor(ScanValues& values, MidiNote note)
{
	values.m_subTracks |= DrumMidi::ParseLaneColor<NoteOn>(*this, note);
	return values.m_subTracks == 31;
}
