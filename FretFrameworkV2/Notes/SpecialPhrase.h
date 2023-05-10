#pragma once
#include "Sustained.h"

enum class SpecialPhraseType
{
	FaceOff_Player1 = 0,
	FaceOff_Player2 = 1,
	StarPower = 2,
	Solo = 3,
	LyricLine = 4,
	RangeShift = 5,
	HarmonyLine = 6,
	Arpeggio = 7,
	StarPower_Diff = 8,

	StarPowerActivation = 64,
	Tremolo = 65,
	Trill = 66,
	LyricShift = 67,
	Chord_Numbering = 68,
};

struct SpecialPhrase : public Sustained
{
	SpecialPhraseType type;
	constexpr SpecialPhrase(SpecialPhraseType _type, uint64_t length) : type(_type) { setLength(length); }
};
