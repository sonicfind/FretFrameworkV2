#pragma once
#include "Midi_Loader_Instrument.h"
#include "ProKeysDifficulty.h"

template <>
struct Midi_Loader_Instrument::Loader_Lanes<Keys_Pro>
{
	uint64_t values[25] =
	{
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
	};
};

template <>
struct Midi_Loader_Instrument::Loader_Diff<Keys_Pro>
{
	static constexpr std::pair<unsigned char, unsigned char> NOTERANGE{ 48, 72 };
};

template <>
Midi_Loader_Instrument::Loader<Keys_Pro>::Loader(InstrumentalTrack<Keys_Pro>& track, unsigned char multiplierNote);

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<Keys_Pro>::parseBRE(uint32_t midiValue) {}

template<>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<Keys_Pro>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t lane = note.value - m_difficulties->NOTERANGE.first;
	if constexpr (NoteOn)
	{
		m_lanes.values[lane] = m_position;
		ConstructNote(m_track[0], m_position);
	}
	else
	{
		uint64_t colorPosition = m_lanes.values[lane];
		if (colorPosition != UINT64_MAX)
		{
			Midi_Loader::GetNode(m_track[0].m_notes, colorPosition)->set({ (char)note.value, m_position - colorPosition });
			m_lanes.values[lane] = UINT64_MAX;
		}
	}
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<Keys_Pro>::toggleExtraValues(MidiNote note)
{
	if constexpr (!NoteOn)
		return;

	size_t index;
	switch (note.value)
	{
	case 0: index = 0; break;
	case 2: index = 1; break;
	case 4: index = 2; break;
	case 5: index = 3; break;
	case 7: index = 4; break;
	case 9: index = 5; break;
	default:
		return;
	}
	static constexpr ProKey_Ranges RANGES[] = { ProKey_Ranges::C1_E2, ProKey_Ranges::D1_F2, ProKey_Ranges::E1_G2, ProKey_Ranges::F1_A2, ProKey_Ranges::G1_B2, ProKey_Ranges::A1_C3, };
	m_track[0].m_ranges.get_or_emplace_back(m_position) = RANGES[index];
}
