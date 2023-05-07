#pragma once
#include "DrumNote.h"

class DrumNote_Legacy : public DrumNote<5, true>
{
public:
	template <size_t numPads, bool PRO_DRUMS>
	DrumNote<numPads, PRO_DRUMS> transformNote() const
	{
		static_assert(numPads <= 5);

		DrumNote<numPads, PRO_DRUMS> note;

		if (m_special.isActive())
			note.getSpecial() = std::move(m_special);
		else if (m_doubleBass.isActive())
			note.getDoubleBass() = std::move(m_doubleBass);

		for (size_t i = 0; i < numPads; ++i)
			if (m_colors[i].isActive())
			{
				auto& col = note.get(i);
				col.set(m_colors[i].getSustain());
				col.setDynamics(m_colors[i].getDynamics());
				if constexpr (PRO_DRUMS)
					col.setCymbal(m_colors[i].isCymbal());
			}

		note.setFlam(isFlammed());
		return note;
	}

	DrumType_Enum evaluateDrumType() const noexcept
	{
		if (m_colors[4].isActive())
			return DrumType_Enum::FIVELANE;
		else if (m_colors[1].isCymbal() || m_colors[2].isCymbal() || m_colors[3].isCymbal())
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
