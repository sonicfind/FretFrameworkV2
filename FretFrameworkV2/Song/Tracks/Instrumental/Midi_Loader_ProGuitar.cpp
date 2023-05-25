#include "Midi_Loader_ProGuitar.h"

Midi_Loader::Loader_Phrases<SpecialPhraseType> GetPhrases(unsigned char multiplierNote)
{
	return { {
	{ { 115 }, { SpecialPhraseType::Solo } },
	{ { multiplierNote }, { SpecialPhraseType::StarPower } },
	{ { 126 }, { SpecialPhraseType::Tremolo } },
	{ { 127 }, { SpecialPhraseType::Trill } },
	} };
};

template <>
Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::Loader(InstrumentalTrack<GuitarNote_Pro<17>>& track, unsigned char multiplierNote)
	: m_track(track), m_phrases(GetPhrases(multiplierNote)) {}

template <>
Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::Loader(InstrumentalTrack<GuitarNote_Pro<22>>& track, unsigned char multiplierNote)
	: m_track(track), m_phrases(GetPhrases(multiplierNote)) {}

size_t Midi_Loader_ProGuitar::DIFF(size_t noteValue)
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
size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::getDifficulty(size_t noteValue) const noexcept { return Midi_Loader_ProGuitar::DIFF(noteValue); }

template <>
size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::getDifficulty(size_t noteValue) const noexcept { return Midi_Loader_ProGuitar::DIFF(noteValue); }
