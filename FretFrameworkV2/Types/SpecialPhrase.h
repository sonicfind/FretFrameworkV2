#pragma once
#include <stdint.h>
#include <fstream>

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

class SpecialPhrase
{
public:
	constexpr SpecialPhrase(SpecialPhraseType type, uint32_t duration = 1) : m_type(type), m_duration(duration) {}

	void setType(SpecialPhraseType type) { m_type = type; }
	void setDuration(uint32_t duration) { m_duration = duration > 0 ? duration : 1; }

	SpecialPhraseType getType() const { return m_type; }

	uint32_t getDuration() const
	{
		return m_duration;
	}
	void operator*=(float multiplier);

private:
	SpecialPhraseType m_type;
	uint32_t m_duration = 0;
};
