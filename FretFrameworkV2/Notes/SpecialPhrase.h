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

	StarPowerActivation_or_BRE = 64,
	Tremolo = 65,
	Trill = 66,
	LyricShift = 67,
};

struct SpecialPhrase : public Sustained<false>
{
	SpecialPhraseType type;
	unsigned char midiVelocity;
	constexpr SpecialPhrase(SpecialPhraseType _type, uint64_t length, unsigned char velocity = 100) : type(_type), midiVelocity(velocity) { setLength(length); }
};
