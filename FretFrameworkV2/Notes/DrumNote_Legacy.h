#pragma once
#include "DrumNote.h"

enum class DrumType_Enum
{
	LEGACY,
	FOURLANE_PRO,
	FIVELANE,
};

class DrumNote_Legacy : public DrumNote<5, true>
{
public:
	template <size_t numPads, bool PRO_DRUMS>
	DrumNote<numPads, PRO_DRUMS> transformNote() const
	{
		static_assert(numPads <= 5);

		DrumNote<numPads, PRO_DRUMS> note;
		memcpy(&note, this, sizeof(numPads * sizeof(DrumPad)));
		if (m_special.isActive())
			note.set(0, m_special.getSustain());

		note.setDoubleBass(isDoubleBass());
		note.setFlam(isFlammed());
		return note;
	}

	static bool testCymbal_V1(size_t lane)
	{
		return lane >= 66 && lane <= 68;
	}

	static DrumType_Enum testDrumType_V1(size_t lane)
	{
		if (testCymbal_V1(lane))
			return DrumType_Enum::FOURLANE_PRO;
		else if (lane == 5)
			return DrumType_Enum::FIVELANE;
		else
			return DrumType_Enum::LEGACY;
	}
};
