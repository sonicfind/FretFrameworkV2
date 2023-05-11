#pragma once
#include "Notes.h"

template <size_t numColors>
class Keys : public Note<Sustained, numColors>
{
public:
	using Note<Sustained, numColors>::m_colors;

	bool set_V1(const size_t lane, uint64_t sustain)
	{
		if (lane >= numColors)
			return false;

		m_colors[lane].setLength(sustain);
		return true;
	}

	static bool TestIndex_V1(const size_t lane)
	{
		return lane < numColors;
	}
};
