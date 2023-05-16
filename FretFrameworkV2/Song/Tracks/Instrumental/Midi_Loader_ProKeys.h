#pragma once
#include "Midi_Loader_Instrument.h"
#include "ProKeysDifficulty.h"

template <>
struct Midi_Loader_Instrument::Loader_Lanes<Keys_Pro> {};

template <>
struct Midi_Loader_Instrument::Loader_Diff<Keys_Pro> {};

template <>
struct Midi_Loader_Instrument::Loader_Ext<Keys_Pro>
{
	uint64_t notes[25] =
	{
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
	};
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<Keys_Pro>::s_noteRange{ 48, 72 };

template<>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<Keys_Pro>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t lane = note.value - s_noteRange.first;
	if constexpr (NoteOn)
	{
		m_ext.notes[lane] = m_position;
		ConstructNote(m_track[0], m_position);
	}
	else
	{
		uint64_t colorPosition = m_ext.notes[lane];
		if (colorPosition != UINT64_MAX)
		{
			Midi_Loader::GetNode(m_track[0].m_notes, colorPosition).object.set({ (char)note.value, m_position - colorPosition });
			m_ext.notes[lane] = UINT64_MAX;
		}
	}
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<Keys_Pro>::toggleExtraValues(MidiNote note)
{
	if constexpr (!NoteOn)
		return;

	switch (note.value)
	{
	case 0: m_track[0].m_ranges.get_or_emplace_back(m_position) = ProKey_Ranges::C1_E2; break;
	case 2: m_track[0].m_ranges.get_or_emplace_back(m_position) = ProKey_Ranges::D1_F2; break;
	case 4: m_track[0].m_ranges.get_or_emplace_back(m_position) = ProKey_Ranges::E1_G2; break;
	case 5: m_track[0].m_ranges.get_or_emplace_back(m_position) = ProKey_Ranges::F1_A2; break;
	case 7: m_track[0].m_ranges.get_or_emplace_back(m_position) = ProKey_Ranges::G1_B2; break;
	case 9: m_track[0].m_ranges.get_or_emplace_back(m_position) = ProKey_Ranges::A1_C3; break;
	}
}
