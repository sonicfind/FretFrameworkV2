#include "GuitarNote.h"

template<>
bool GuitarNote<5>::set_V1(const size_t lane, uint32_t sustain)
{
	if (lane < 5)
	{
		m_colors[lane].set(sustain);
		m_special = REPLACEMENTS[0];
	}
	else if (lane == 5)
		m_forcing = ForceStatus::FORCED;
	else if (lane == 6)
		toggleTap();
	else if (lane == 7)
	{
		m_special.set(sustain);
		memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
	}
	else
		return false;

	return true;
}

bool GuitarNote<6>::set_V1(const size_t lane, uint32_t sustain)
{
	if (lane < 5)
	{
		static constexpr size_t lanes[5] = { 3, 4, 5, 0, 1 };
		m_colors[lanes[lane]].set(sustain);
		m_special = REPLACEMENTS[0];
	}
	else if (lane == 5)
		m_forcing = ForceStatus::FORCED;
	else if (lane == 6)
		toggleTap();
	else if (lane == 7)
	{
		m_special.set(sustain);
		memcpy(m_colors, REPLACEMENTS, sizeof(m_colors));
	}
	else if (lane == 8)
		m_colors[2].set(sustain);
	else
		return false;
	return true;
}

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<5>::getMidiNotes() const noexcept
{
	auto colors = Note_withSpecial<NoteColor, 5, NoteColor>::getMidiNotes();
	for (auto& color : colors)
		std::get<0>(color)--;
	return colors;
}

template<>
std::vector<std::tuple<char, char, uint32_t>> GuitarNote<6>::getMidiNotes() const noexcept
{
	static constexpr char lanes[7] = { -2, 2, 3, 4, -1, 0, 1 };
	auto colors = Note_withSpecial<NoteColor, 6, NoteColor>::getMidiNotes();
	for (auto& color : colors)
		std::get<0>(color) = lanes[std::get<0>(color)];
	return colors;
}
