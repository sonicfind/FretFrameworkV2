#pragma once
#include <vector>
#include "NoteColor.h"

template <class NoteType, size_t numColors>
class Note
{
protected:
	NoteType m_colors[numColors];

public:
	bool modify(char modifier, size_t lane) { return false; }

	bool set(const size_t lane, uint32_t sustain)
	{
		if (lane == 0 || lane > numColors)
			return false;

		m_colors[lane - 1].set(sustain);
		return true;
	}

	NoteType& getColor(const size_t lane)
	{
		if (lane >= numColors)
			throw std::runtime_error("Invalid index");
		return m_colors[lane];
	}

	const NoteType& getColor(const size_t lane) const
	{
		if (lane >= numColors)
			throw std::runtime_error("Invalid index");
		return m_colors[lane];
	}

	std::vector<std::pair<size_t, uint32_t>> getActiveColors() const
	{
		std::vector<std::pair<size_t, uint32_t>> activeColors;
		for (size_t i = 0; i < numColors; ++i)
			if (m_colors[i].isActive())
				activeColors.push_back({ i + 1, m_colors[i].getSustain() });
		return activeColors;
	}

	std::vector<std::pair<char, size_t>> getActiveModifiers() const
	{
		return {};
	}

	std::vector<char> getActiveModifiers(size_t index) const
	{
		return {};
	}

	uint32_t getLongestSustain() const noexcept
	{
		uint32_t sustain = 0;
		for (const auto& color : m_colors)
			if (color.isActive() && color.getSustain() > sustain)
				sustain = color.getSustain();
		return sustain;
	}

	std::vector<std::tuple<char, char, uint32_t>> getMidiNotes() const noexcept
	{
		auto base = getActiveColors();
		std::vector<std::tuple<char, char, uint32_t>>  colors;
		for (const auto& col : base)
			colors.push_back({ char(col.first), 100, col.second });
		return colors;
	}

	bool operator==(const Note& note) const
	{
		for (int i = 0; i < numColors; ++i)
			if (m_colors[i] != note.m_colors[i])
				return false;
		return true;
	}

	bool operator!=(const Note& note) const
	{
		return !operator==(note);
	}

	void operator*=(float multiplier)
	{
		for (auto& col : m_colors)
			col *= multiplier;
	}

	static constexpr size_t GetLaneCount()
	{
		return numColors;
	}

	static bool TestIndex(size_t lane)
	{
		return lane != 0 && lane <= numColors;
	}
};
