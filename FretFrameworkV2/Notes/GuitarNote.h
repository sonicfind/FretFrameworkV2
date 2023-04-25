#pragma once
#include "Notes_Special.h"
#include <assert.h>

enum class ForceStatus
{
	UNFORCED,
	FORCED,
	HOPO_ON,
	HOPO_OFF
};

template <size_t numColors>
class GuitarNote : public Note_withSpecial<NoteColor, numColors, NoteColor>
{
	using Note_withSpecial<NoteColor, numColors, NoteColor>::m_colors;
	using Note_withSpecial<NoteColor, numColors, NoteColor>::m_special;

	ForceStatus m_forcing = ForceStatus::UNFORCED;
	bool m_isTap = false;
	static constexpr NoteColor REPLACEMENTS[numColors]{};

public:
	bool set(const size_t lane, uint32_t sustain)
	{
		if (!Note_withSpecial<NoteColor, numColors, NoteColor>::set(lane, sustain))
			return false;

		if (lane == 0)
			memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
		else
			m_special = REPLACEMENTS[0];
		return true;
	}

	bool set_V1(const size_t lane, uint32_t sustain)
	{
		if (lane > numColors + 2) [[unlikely]]
			return false;

		if (sustain < 20)
			sustain = 1;

		auto testExtension = [&]() -> bool
		{
			if constexpr (numColors <= 5)
				return false;

			if (lane == 8)
				m_colors[2].set(sustain);
			else if (lane > 8)
				m_colors[lane - 3].set(sustain);
			else
				return false;
			return true;
		};

		if (lane < 5)
		{
			if constexpr (numColors > 5)
			{
				static constexpr size_t lanes[5] = { 3, 4, 5, 0, 1 };
				m_colors[lanes[lane]].set(sustain);
			}
			else
				m_colors[lane].set(sustain);
			m_special = REPLACEMENTS[0];
		}
		else if (!testExtension())
		{
			if (lane == 7)
			{
				m_special.set(sustain);
				memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
			}
			else if (lane == 5)
				setForcing(ForceStatus::FORCED);
			else
				toggleTap();
		}

		return true;
	}

	bool modify(char modifier, size_t lane = 0)
	{
		switch (modifier)
		{
		case 'T':
			toggleTap();
			return true;
		case 'F':
			switch (m_forcing)
			{
			case ForceStatus::UNFORCED:
				setForcing(ForceStatus::FORCED);
				break;
			default:
				setForcing(ForceStatus::UNFORCED);
			}
			return true;
		case '<':
			setForcing(ForceStatus::HOPO_ON);
			return true;
		case '>':
			setForcing(ForceStatus::HOPO_OFF);
			return true;
		}
		return false;
	}

	std::vector<std::pair<size_t, uint32_t>> getActiveColors() const
	{
		if (m_special.isActive())
			return { { 0, m_special.getSustain() } };
		else
			return Note<NoteColor, numColors>::getActiveColors();
	}

	std::vector<std::pair<char, size_t>> getActiveModifiers() const
	{
		std::vector<std::pair<char, size_t>> modifiers;
		switch (m_forcing)
		{
		case ForceStatus::FORCED:
			modifiers.push_back({ 'F', SIZE_MAX });
			break;
		case ForceStatus::HOPO_ON:
			modifiers.push_back({ '<', SIZE_MAX });
			break;
		case ForceStatus::HOPO_OFF:
			modifiers.push_back({ '>', SIZE_MAX });
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
		case ForceStatus::FORCED:
			modifiers.push_back('F');
			break;
		case ForceStatus::HOPO_ON:
			modifiers.push_back('<');
			break;
		case ForceStatus::HOPO_OFF:
			modifiers.push_back('>');
			break;
		}

		if (m_isTap)
			modifiers.push_back('T');
		return modifiers;
	}

	uint32_t getLongestSustain() const
	{
		if (m_special)
			return m_special.getSustain();
		else
		{
			uint32_t sustain = 0;
			for (const auto& color : m_colors)
				if (color.isActive() && color.getSustain() > sustain)
					sustain = color.getSustain();
			return sustain;
		}
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
		m_forcing = status;
	}

	[[nodiscard]] bool isTapped() const noexcept { return m_isTap; }
	[[nodiscard]] ForceStatus getForcing() const noexcept { return m_forcing; }

	static bool testIndex_V1(const size_t lane)
	{
		static constexpr bool validLanes[256] = { true, true, true, true, true, false, false, true, numColors == 6 };
		assert(lane < 256);
		return validLanes[lane];
	}
};
