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

	StarPowerActivation = 64,
	Tremolo = 65,
	Trill = 66,
	LyricShift = 67,
	Chord_Numbering = 68,
};

class SpecialPhrase
{
public:
	constexpr SpecialPhrase(SpecialPhraseType type, uint32_t duration = 0) : m_type(type), m_duration(duration) {}
	char getMidiNote() const;

	void setType(SpecialPhraseType type) { m_type = type; }
	void setDuration(uint32_t duration) { m_duration = duration; }

	SpecialPhraseType getType() const { return m_type; }

	uint32_t getDuration() const
	{
		return m_duration;
	}

	uint32_t getMidiDuration() const
	{
		if (m_duration > 0)
			return m_duration;
		return 1;
	}
	void operator*=(float multiplier);

private:
	SpecialPhraseType m_type;
	uint32_t m_duration = 0;
};
