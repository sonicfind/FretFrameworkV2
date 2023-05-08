#pragma once
#include "DrumScan_Legacy.h"
#include "InstrumentalScan_Midi.h"

template<>
struct Midi_Scanner_Extensions<DrumNote_Legacy>
{
	bool expertPlus = false;
	bool doubleBass = false;
	DrumType_Enum type = DrumType_Enum::LEGACY;
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote_Legacy>::parseLaneColor(MidiNote note)
{
	if (note.value == 95)
	{
		if (m_ext.expertPlus)
			return false;

		if constexpr (!NoteOn)
		{
			if (m_ext.doubleBass)
			{
				m_values.addSubTrack(4);
				m_ext.expertPlus = true;
			}
		}
		m_ext.doubleBass = true;
	}
	else
	{
		const int noteValue = note.value - Midi_Scanner::s_noteRange.first;
		const int diff = Midi_Scanner::s_diffValues[noteValue];
		const int lane = m_laneValues[noteValue];
		if (lane == 5 && m_ext.type == DrumType_Enum::LEGACY)
			m_ext.type = DrumType_Enum::FIVELANE;

		if (!m_difficulties[diff].active)
		{
			if (lane < 6)
			{
				if constexpr (!NoteOn)
				{
					if (m_difficulties[diff].notes[lane])
					{
						m_values.addSubTrack(diff);
						m_difficulties[diff].active = true;
					}
				}
				m_difficulties[diff].notes[lane] = true;
			}
		}
	}
	return m_values.m_subTracks == 31 && m_ext.type != DrumType_Enum::LEGACY;
}

template <>
bool Midi_Scanner<DrumNote_Legacy>::processExtraValues(MidiNote note);

class DrumScan_Legacy_Midi : public DrumScan_Legacy
{
public:
	DrumScan_Legacy_Midi(MidiFileReader& reader);
};
