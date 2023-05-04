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
		return Note_withSpecial<NoteColor, numColors, NoteColor>::getMidiNotes();
	}

	uint32_t getLongestSustain() const noexcept
	{
		if (m_special.isActive())
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
		if (status == ForceStatus::FORCED)
			throw std::runtime_error("Don't you dare fucking try setting this to FORCED");
		m_forcing = status;
	}

	[[nodiscard]] bool isTapped() const noexcept { return m_isTap; }
	[[nodiscard]] ForceStatus getForcing() const noexcept { return m_forcing; }

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
bool GuitarNote<5>::set_V1(const size_t lane, uint32_t sustain);

template<>
bool GuitarNote<6>::set_V1(const size_t lane, uint32_t sustain);

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<5>::getMidiNotes() const noexcept;

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<6>::getMidiNotes() const noexcept;
