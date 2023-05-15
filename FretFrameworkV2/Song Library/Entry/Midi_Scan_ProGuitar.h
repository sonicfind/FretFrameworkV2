#pragma once
#include "Midi_Scan_Instrument.h"
#include "Notes/GuitarNote_Pro.h"

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<17>>::s_noteRange{ 24, 106 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<22>>::s_noteRange{ 24, 106 };

template <>
constexpr size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<17>>::s_diffValues[96] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

template <>
constexpr size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<22>>::s_diffValues[96] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

template <>
Midi_Scanner_Instrument::Scanner_Lanes<GuitarNote_Pro<17>>::Scanner_Lanes();

template <>
Midi_Scanner_Instrument::Scanner_Lanes<GuitarNote_Pro<22>>::Scanner_Lanes();

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<17>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	if (channel == 1)
		return;

	const size_t noteValue = note.value - s_noteRange.first;
	const size_t diff = getDifficulty(noteValue);
	if (m_difficulties[diff].active)
		return;

	const size_t lane = m_lanes.values[noteValue];
	if (lane >= 6)
		return;

	if constexpr (NoteOn)
	{
		if (note.velocity <= 117)
			m_difficulties[diff].notes[lane] = true;
	}
	else if (m_difficulties[diff].notes[lane])
	{
		m_scan.addDifficulty(diff);
		m_difficulties[diff].active = true;
	}
}

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<22>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	if (channel == 1)
		return;

	const size_t noteValue = note.value - s_noteRange.first;
	const size_t diff = getDifficulty(noteValue);
	if (m_difficulties[diff].active)
		return;

	const size_t lane = m_lanes.values[noteValue];
	if (lane >= 6)
		return;

	if constexpr (NoteOn)
	{
		if (note.velocity <= 122)
			m_difficulties[diff].notes[lane] = true;
	}
	else if (m_difficulties[diff].notes[lane])
	{
		m_scan.addDifficulty(diff);
		m_difficulties[diff].active = true;
	}
}
