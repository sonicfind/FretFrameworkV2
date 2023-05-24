#pragma once
#include "Midi_Scan_Instrument.h"
#include "Notes/GuitarNote_Pro.h"

template <int numStrings, int numFrets>
struct Midi_Scanner_Instrument::Scanner_Lanes<GuitarNote_Pro<numStrings, numFrets>> {};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner_Diff<GuitarNote_Pro<6, 17>>::NOTERANGE{ 24, 106 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner_Diff<GuitarNote_Pro<4, 17>>::NOTERANGE{ 24, 106 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner_Diff<GuitarNote_Pro<6, 22>>::NOTERANGE{ 24, 106 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Scanner_Instrument::Scanner_Diff<GuitarNote_Pro<4, 22>>::NOTERANGE{ 24, 106 };

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<6, 17>>::getDifficulty(size_t noteValue) const noexcept;

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<4, 17>>::getDifficulty(size_t noteValue) const noexcept;

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<6, 22>>::getDifficulty(size_t noteValue) const noexcept;

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<4, 22>>::getDifficulty(size_t noteValue) const noexcept;

namespace Midi_Scan_ProGuitar
{
	constexpr size_t LANES[96] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	};

	size_t DIFF(size_t noteValue);

	template <bool NoteOn, int numStrings, int numFrets>
	bool ParseColor(Midi_Scanner_Instrument::Scanner_Diff<GuitarNote_Pro<numStrings, numFrets>>& diff, size_t noteValue, unsigned char channel, unsigned char velocity)
	{
		if (!diff.active && channel != 1)
		{
			const size_t lane = LANES[noteValue];
			if (lane < 6)
			{
				if constexpr (!NoteOn)
				{
					if (diff.notes[lane])
					{
						diff.active = true;
						return true;
					}
				}
				else if (velocity <= 117)
					diff.notes[lane] = true;
			}
		}
		return false;
	}
}

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<6, 17>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - m_difficulties->NOTERANGE.first;
	const size_t diff = getDifficulty(noteValue);
	if (Midi_Scan_ProGuitar::ParseColor<NoteOn>(m_difficulties[diff], noteValue, channel, note.velocity))
		m_scan.addDifficulty(diff);
}

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<4, 17>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - m_difficulties->NOTERANGE.first;
	const size_t diff = getDifficulty(noteValue);
	if (Midi_Scan_ProGuitar::ParseColor<NoteOn>(m_difficulties[diff], noteValue, channel, note.velocity))
		m_scan.addDifficulty(diff);
}

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<6, 22>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - m_difficulties->NOTERANGE.first;
	const size_t diff = getDifficulty(noteValue);
	if (Midi_Scan_ProGuitar::ParseColor<NoteOn>(m_difficulties[diff], noteValue, channel, note.velocity))
		m_scan.addDifficulty(diff);
}

template<>
template <bool NoteOn>
void Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<4, 22>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - m_difficulties->NOTERANGE.first;
	const size_t diff = getDifficulty(noteValue);
	if (Midi_Scan_ProGuitar::ParseColor<NoteOn>(m_difficulties[diff], noteValue, channel, note.velocity))
		m_scan.addDifficulty(diff);
}
