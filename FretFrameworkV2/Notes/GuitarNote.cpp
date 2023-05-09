#include "GuitarNote.h"

template<>
bool GuitarNote<5>::set_V1(const size_t lane, uint32_t length)
{
	if (lane < 5)
	{
		m_colors[lane].setLength(length);
		m_special = REPLACEMENTS[0];
	}
	else if (lane == 5)
		m_forcing = ForceStatus::FORCED;
	else if (lane == 6)
		toggleTap();
	else if (lane == 7)
	{
		m_special.setLength(length);
		memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
	}
	else
		return false;

	return true;
}

bool GuitarNote<6>::set_V1(const size_t lane, uint32_t length)
{
	if (lane < 5)
	{
		static constexpr size_t lanes[5] = { 3, 4, 5, 0, 1 };
		m_colors[lanes[lane]].setLength(length);
		m_special = REPLACEMENTS[0];
	}
	else if (lane == 5)
		m_forcing = ForceStatus::FORCED;
	else if (lane == 6)
		toggleTap();
	else if (lane == 7)
	{
		m_special.setLength(length);
		memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
	}
	else if (lane == 8)
		m_colors[2].setLength(length);
	else
		return false;
	return true;
}

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<5>::getMidiNotes() const noexcept
{
	if (m_special.isActive())
		return { {0, 100, m_special.getLength()} };
	else
		return Note<Sustained, 5>::getMidiNotes();
}

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<6>::getMidiNotes() const noexcept
{
	static constexpr char lanes[7] = { -2, 2, 3, 4, -1, 0, 1 };
	auto colors = Note_withSpecial<Sustained, 6, Sustained>::getMidiNotes();
	for (auto& color : colors)
		std::get<0>(color) = lanes[std::get<0>(color)];
	return colors;
}
