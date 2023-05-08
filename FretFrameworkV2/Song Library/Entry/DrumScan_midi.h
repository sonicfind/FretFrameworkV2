#pragma once
#include "DrumScan.h"
#include "Notes/DrumNote_Legacy.h"
#include "InstrumentalScan_Midi.h"

template<size_t numPads, bool PRO_DRUMS>
struct Midi_Scanner_Extensions<DrumNote<numPads, PRO_DRUMS>>
{
	bool expertPlus = false;
	bool doubleBass = false;
};

template<>
struct Midi_Scanner_Extensions<DrumNote_Legacy>
{
	bool expertPlus = false;
	bool doubleBass = false;
	DrumType_Enum type = DrumType_Enum::LEGACY;
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner<DrumNote<5, false>>::s_noteRange{ 60, 102 };


template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template <>
bool Midi_Scanner<DrumNote<4, true>>::isFinished() const noexcept;

template <>
bool Midi_Scanner<DrumNote<5, false>>::isFinished() const noexcept;

template <>
bool Midi_Scanner<DrumNote_Legacy>::isFinished() const noexcept;

template <bool NoteOn, class T>
bool ParseSpec(Midi_Scanner<T>& scanner, MidiNote note)
{
	if (note.value != 95)
		return false;

	if (!scanner.m_ext.expertPlus)
	{
		if constexpr (!NoteOn)
		{
			if (scanner.m_ext.doubleBass)
				scanner.m_ext.expertPlus = true;
		}
		else
			scanner.m_ext.doubleBass = true;
	}
	return true;
}

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote<4, true>>::processSpecialNote(MidiNote note)
{
	return ParseSpec<NoteOn>(*this, note);
}

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote<5, false>>::processSpecialNote(MidiNote note)
{
	return ParseSpec<NoteOn>(*this, note);
}

template <>
template <bool NoteOn>
bool Midi_Scanner<DrumNote_Legacy>::processSpecialNote(MidiNote note)
{
	return ParseSpec<NoteOn>(*this, note);
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
