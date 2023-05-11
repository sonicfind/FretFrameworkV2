#pragma once
#include "DrumNote.h"

class DrumNote_Legacy : public DrumNote<DrumPad_Pro, 5>
{
	static DrumType_Enum s_currentType;
public:
	bool set_V1(const size_t lane, uint64_t length)
	{
		if (!DrumNote<DrumPad_Pro, 5>::set_V1(lane, length))
			return false;

		if (s_currentType == DrumType_Enum::LEGACY)
			s_currentType = EvaluateDrumType(lane);
		return true;
	}

	DrumPad_Pro& get(const size_t lane)
	{
		if (s_currentType == DrumType_Enum::LEGACY && lane == 4)
			s_currentType = DrumType_Enum::FIVELANE;
		return m_colors[lane];
	}

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
				col.setLength(m_colors[i].getLength());
				col.setDynamics(m_colors[i].getDynamics());
				if constexpr (std::is_same<DrumType, DrumPad_Pro>::value)
					col.setCymbal(m_colors[i].isCymbal());
			}

		note.setFlam(isFlammed());
		return note;
	}

	static void Signal4Pro()
	{
		if (s_currentType == DrumType_Enum::LEGACY)
			s_currentType = DrumType_Enum::FOURLANE_PRO;
	}

	static void ResetType() { s_currentType = DrumType_Enum::LEGACY; }
	static DrumType_Enum GetType() { return s_currentType; }

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
