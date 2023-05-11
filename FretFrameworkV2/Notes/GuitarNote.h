#pragma once
#include "Notes_Special.h"
#include <stdexcept>

enum class ForceStatus
{
	UNFORCED,
	FORCED,
	HOPO_ON,
	HOPO_OFF
};

template <size_t numColors>
class GuitarNote : public Note_withSpecial<Sustained, numColors, Sustained>
{
	using Note_withSpecial<Sustained, numColors, Sustained>::m_colors;
	using Note_withSpecial<Sustained, numColors, Sustained>::m_special;

	ForceStatus m_forcing = ForceStatus::UNFORCED;
	bool m_isTap = false;
	static constexpr Sustained REPLACEMENTS[numColors]{};

public:
	bool set(const size_t lane, uint64_t length)
	{
		if (!Note_withSpecial<Sustained, numColors, Sustained>::set(lane, length))
			return false;

		if (lane == 0)
			memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
		else
			m_special = REPLACEMENTS[0];
		return true;
	}

	bool set_V1(const size_t lane, uint64_t length)
	{
		return false;
	}

	bool modify(char modifier, size_t lane = 0)
	{
		switch (modifier)
		{
		case 'T':
			toggleTap();
			return true;
		case 'F':
			if (m_forcing == ForceStatus::UNFORCED)
				m_forcing = ForceStatus::FORCED;
			return true;
		case 'H':
			setForcing(ForceStatus::HOPO_ON);
			return true;
		case 'S':
			setForcing(ForceStatus::HOPO_OFF);
			return true;
		}
		return false;
	}

	std::vector<std::pair<char, size_t>> getActiveModifiers() const
	{
		std::vector<std::pair<char, size_t>> modifiers;
		switch (m_forcing)
		{
		case ForceStatus::HOPO_ON:
			modifiers.push_back({ 'H', SIZE_MAX });
			break;
		case ForceStatus::HOPO_OFF:
			modifiers.push_back({ 'S', SIZE_MAX });
			break;
		}

		if (m_isTap)
			modifiers.push_back({ 'T', SIZE_MAX });
		return modifiers;
	}

	std::vector<char> getActiveModifiers(size_t index) const
	{
		std::vector<char> modifiers;
		switch (m_forcing)
		{
		case ForceStatus::HOPO_ON:
			modifiers.push_back('H');
			break;
		case ForceStatus::HOPO_OFF:
			modifiers.push_back('S');
			break;
		}

		if (m_isTap)
			modifiers.push_back('T');
		return modifiers;
	}

	std::vector<std::tuple<char, char, uint32_t>> getMidiNotes() const noexcept
	{
		return Note_withSpecial<Sustained, numColors, Sustained>::getMidiNotes();
	}

	uint64_t getLongestSustain() const noexcept
	{
		if (m_special.isActive())
			return m_special.getLength();
		else
			return Note<Sustained, numColors>::getLongestSustain();
	}

	void toggleTap()
	{
		m_isTap = !m_isTap;
	}

	void setTap(bool enable)
	{
		m_isTap = enable;
	}

	void setForcing(ForceStatus status)
	{
		if (status == ForceStatus::FORCED)
			throw std::runtime_error("Don't you dare fucking try setting this to FORCED");
		m_forcing = status;
	}

	[[nodiscard]] bool isTapped() const noexcept { return m_isTap; }
	[[nodiscard]] ForceStatus getForcing() const noexcept { return m_forcing; }

	bool hasSameFretting(const GuitarNote& note) const noexcept
	{
		if (m_special.isActive())
			return note.m_special.isActive();

		for (size_t i = 0; i < numColors; ++i)
			if (m_colors[i].isActive() != note.m_colors[i].isActive())
				return false;
		return true;
	}

	bool isChorded() const noexcept
	{
		if (m_special.isActive())
			return false;

		size_t num = 0;
		for (const auto& col : m_colors)
			num += col.isActive();
		return num > 1;
	}

	// Assumes current note is a single fret
	bool isContainedIn(const GuitarNote& note) const noexcept
	{
		for (size_t i = 0; i < numColors; ++i)
			if (m_colors[i].isActive())
				return note.m_colors[i].isActive();
		return false;
	}

	static bool TestIndex_V1(const size_t lane)
	{
		struct ValidArray
		{
			bool lanes[256] = { true, true, true, true, true, false, false, true };
			constexpr ValidArray()
			{
				for (size_t i = 6; i <= numColors && i < 254; ++i)
					lanes[i + 2] = true;
			}
		} arr;
		return lane < 256 && arr.lanes[lane];
	}
};

template<>
bool GuitarNote<5>::set_V1(const size_t lane, uint64_t sustain);

template<>
bool GuitarNote<6>::set_V1(const size_t lane, uint64_t sustain);

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<5>::getMidiNotes() const noexcept;

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<6>::getMidiNotes() const noexcept;
