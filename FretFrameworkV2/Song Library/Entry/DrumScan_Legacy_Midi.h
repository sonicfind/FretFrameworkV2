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
bool Midi_Scanner<DrumNote_Legacy>::isFinished() const noexcept;

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote_Legacy>::processSpecialNote(MidiNote note)
{
	if (note.value != 95)
		return false;

	if (!m_ext.expertPlus)
	{
		if constexpr (!NoteOn)
		{
			if (m_ext.doubleBass)
				m_ext.expertPlus = true;
		}
		else
			m_ext.doubleBass = true;
	}
	return true;
}

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote_Legacy>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - Midi_Scanner::s_noteRange.first;
	const int lane = m_laneValues[noteValue];

	if (lane >= 6)
		return false;

	if (lane == 5 && m_ext.type == DrumType_Enum::LEGACY)
		m_ext.type = DrumType_Enum::FIVELANE;

	const int diff = Midi_Scanner::s_diffValues[noteValue];
	if (!m_difficulties[diff].active)
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
	return true;
}

template <>
bool Midi_Scanner<DrumNote_Legacy>::processExtraValues(MidiNote note);

class DrumScan_Legacy_Midi : public DrumScan_Legacy
{
public:
	DrumScan_Legacy_Midi(ScanValues& values, MidiFileReader& reader);
};
