#pragma once
#include "Notes.h"

template <size_t numColors>
class Keys : public Note<NoteColor, numColors>
{
public:
	using Note<NoteColor, numColors>::m_colors;

	bool set_V1(const size_t lane, uint32_t sustain)
	{
		if (lane >= numColors)
			return false;

		m_colors[lane].set(sustain);
		return true;
	}

	uint32_t getLongestSustain() const
	{
		uint32_t sustain = 0;
		for (const auto& color : m_colors)
			if (color.isActive() && color.getSustain() > sustain)
				sustain = color.getSustain();
		return sustain;
	}

	static bool TestIndex_V1(const size_t lane)
	{
		return lane < numColors;
	}
};
