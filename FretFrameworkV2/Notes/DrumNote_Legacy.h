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

		if (m_special.isActive())
			note.set(0, m_special.getSustain());

		for (size_t i = 0; i < numPads; ++i)
			if (m_colors[i].isActive())
			{
				note.set(i + 1, m_colors[i].getSustain());
				note.setDynamics(i + 1, m_colors[i].getDynamics());
				if constexpr (PRO_DRUMS)
					note.setCymbal(i + 1, m_colors[i].isCymbal());
			}

		note.setDoubleBass(isDoubleBass());
		note.setFlam(isFlammed());
		return note;
	}

	DrumType_Enum evaluateDrumType() const noexcept
	{
		if (m_colors[4].isActive())
			return DrumType_Enum::FIVELANE;
		else if (isCymbal(2) || isCymbal(3) || isCymbal(4))
			return DrumType_Enum::FOURLANE_PRO;
		else
			return DrumType_Enum::LEGACY;
	}

	static DrumType_Enum EvaluateDrumType(size_t index)
	{
		if (index == 5)
			return DrumType_Enum::FIVELANE;
		else if (66 <= index && index <= 68)
			return DrumType_Enum::FOURLANE_PRO;
		else
			return DrumType_Enum::LEGACY;
	}
};
