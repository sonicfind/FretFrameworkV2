#pragma once
#include "ProKeysScan.h"
#include "Midi_Scan_Instrument.h"

template <>
struct Midi_Scanner_Instrument::Scanner_Lanes<Keys_Pro>
{
	bool values[25]{};
};

template <>
struct Midi_Scanner_Instrument::Scanner_Diff<Keys_Pro>
{
	static constexpr std::pair<unsigned char, unsigned char> NOTERANGE{ 48, 72 };
};

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<Keys_Pro>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t lane = note.value - m_difficulties->NOTERANGE.first;
	if constexpr (NoteOn)
		m_lanes.values[lane] = true;
	else if (m_lanes.values[lane])
		m_scan.addDifficulty(0);
}
