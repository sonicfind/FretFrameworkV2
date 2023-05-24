#include "Midi_Scan_ProGuitar.h"

size_t Midi_Scan_ProGuitar::DIFF(size_t noteValue)
{
	static constexpr size_t DIFFS[96] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	};
	return DIFFS[noteValue];
}

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<6, 17>>::getDifficulty(size_t noteValue) const noexcept { return Midi_Scan_ProGuitar::DIFF(noteValue); }

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<4, 17>>::getDifficulty(size_t noteValue) const noexcept { return  Midi_Scan_ProGuitar::DIFF(noteValue); }

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<6, 22>>::getDifficulty(size_t noteValue) const noexcept { return  Midi_Scan_ProGuitar::DIFF(noteValue); }

template <>
size_t Midi_Scanner_Instrument::Scanner<GuitarNote_Pro<4, 22>>::getDifficulty(size_t noteValue) const noexcept { return  Midi_Scan_ProGuitar::DIFF(noteValue); }
