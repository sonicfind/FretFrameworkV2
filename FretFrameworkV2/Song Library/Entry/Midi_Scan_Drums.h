#pragma once
#include "DrumScan_Legacy.h"
#include "Midi_Scan_Instrument.h"

template<class DrumType, size_t numPads>
struct Midi_Scanner_Instrument::Scanner_Extensions<DrumNote<DrumType, numPads>>
{
	bool expertPlus = false;
	bool doubleBass = false;
};

template<>
struct Midi_Scanner_Instrument::Scanner_Extensions<DrumNote_Legacy> : public Scanner_Extensions<DrumNote<DrumPad_Pro, 5>> {};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner<DrumNote<DrumPad, 5>>::s_noteRange{ 60, 102 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template <bool NoteOn, class DrumType, size_t numPads>
bool ParseSpec(Midi_Scanner_Instrument::Scanner_Extensions<DrumNote<DrumType, numPads>>& ext, MidiNote note)
{
	if (note.value != 95)
		return false;

	if (!ext.expertPlus)
	{
		if constexpr (!NoteOn)
		{
			if (ext.doubleBass)
				ext.expertPlus = true;
		}
		else
			ext.doubleBass = true;
	}
	return true;
}

template <>
template <bool NoteOn>
bool Midi_Scanner_Instrument::Scanner<DrumNote<DrumPad_Pro, 4>>::processSpecialNote(MidiNote note)
{
	return ParseSpec<NoteOn>(m_ext, note);
}

template <>
template <bool NoteOn>
bool Midi_Scanner_Instrument::Scanner<DrumNote<DrumPad, 5>>::processSpecialNote(MidiNote note)
{
	return ParseSpec<NoteOn>(m_ext, note);
}

template <>
template <bool NoteOn>
bool Midi_Scanner_Instrument::Scanner<DrumNote_Legacy>::processSpecialNote(MidiNote note)
{
	return ParseSpec<NoteOn>(m_ext, note);
}

template <>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<DrumNote_Legacy>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - s_noteRange.first;
	const size_t lane = m_lanes.values[noteValue];

	if (lane >= 6)
		return;

	if (lane == 5)
		m_scan.setType(DrumType_Enum::FIVELANE);

	const int diff = s_diffValues[noteValue];
	if (!m_difficulties[diff].active)
	{
		if constexpr (!NoteOn)
		{
			if (m_difficulties[diff].notes[lane])
			{
				m_scan.addDifficulty(diff);
				m_difficulties[diff].active = true;
			}
		}
		m_difficulties[diff].notes[lane] = true;
	}
}

template <>
void Midi_Scanner_Instrument::Scanner<DrumNote_Legacy>::processExtraValues(MidiNote note);
