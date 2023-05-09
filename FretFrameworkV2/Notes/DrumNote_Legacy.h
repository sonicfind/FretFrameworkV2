#pragma once
#include "DrumNote.h"

class DrumNote_Legacy : public DrumNote<DrumPad_Pro, 5>
{
public:
	template <class DrumType, size_t numPads>
	DrumNote<DrumType, numPads> transformNote() const
	{
		static_assert(numPads <= 5);

		DrumNote<DrumType, numPads> note;

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
				if constexpr (std::is_same<DrumType, DrumPad_Pro>::value)
					col.setCymbal(m_colors[i].isCymbal());
			}

		note.setFlam(isFlammed());
		return note;
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
