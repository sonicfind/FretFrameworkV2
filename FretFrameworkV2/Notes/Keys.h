#pragma once
#include "Notes.h"

template <size_t numColors>
class Keys : public Note<Sustained, numColors>
{
public:
	using Note<Sustained, numColors>::m_colors;

	bool set_V1(const size_t lane, uint32_t sustain)
	{
		if (lane >= numColors)
			return false;

		m_colors[lane].setLength(sustain);
		return true;
	}

	std::vector<std::tuple<char, char, uint32_t>> getMidiNotes() const noexcept
	{
		std::vector<std::tuple<char, char, uint32_t>> colors;
		for (char i = 0; i < numColors; ++i)
			if (m_colors[i].isActive())
				colors.push_back({ i, 100, m_colors[i].getLength() });
		return colors;
	}

	uint32_t getLongestSustain() const
	{
		uint32_t sustain = 0;
		for (const auto& color : m_colors)
			if (color.isActive() && color.getLength() > sustain)
				sustain = color.getLength();
		return sustain;
	}

	static bool TestIndex_V1(const size_t lane)
	{
		return lane < numColors;
	}
};
