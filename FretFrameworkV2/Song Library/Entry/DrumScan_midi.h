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

template <>
bool Midi_Scanner<DrumNote<4, true>>::isFinished() const noexcept
{
	return m_values.m_subTracks == 31;
}

template <>
bool Midi_Scanner<DrumNote<5, false>>::isFinished() const noexcept
{
	return m_values.m_subTracks == 31;
}

template <bool NoteOn, size_t numPads, bool PRO_DRUMS>
bool ParseSpec(Midi_Scanner<DrumNote<numPads, PRO_DRUMS>>& scanner, MidiNote note)
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
