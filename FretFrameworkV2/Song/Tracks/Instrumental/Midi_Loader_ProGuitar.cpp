#include "Midi_Loader_ProGuitar.h"

template <>
Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::Loader(InstrumentalTrack<GuitarNote_Pro<17>>& track, unsigned char multiplierNote)
	: m_track(track), m_phrases({
	{ { 115 }, { SpecialPhraseType::Solo } },
	{ { multiplierNote }, { SpecialPhraseType::StarPower } },
	{ { 126 }, { SpecialPhraseType::Tremolo } },
	{ { 127 }, { SpecialPhraseType::Trill} },
}) {}

template <>
Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::Loader(InstrumentalTrack<GuitarNote_Pro<22>>& track, unsigned char multiplierNote)
	: m_track(track), m_phrases({
	{ { 115 }, { SpecialPhraseType::Solo } },
	{ { multiplierNote }, { SpecialPhraseType::StarPower } },
	{ { 126 }, { SpecialPhraseType::Tremolo } },
	{ { 127 }, { SpecialPhraseType::Trill} },
}) {}

template <>
size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::getDifficulty(size_t noteValue) const noexcept
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
size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::getDifficulty(size_t noteValue) const noexcept
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
Midi_Loader_Instrument::Loader_Lanes<GuitarNote_Pro<17>>::Loader_Lanes() : values{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
} {}

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote_Pro<22>>::Loader_Lanes() : values{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
} {}
