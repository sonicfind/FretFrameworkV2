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
		setForcing(ForceStatus::FORCED);
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
		setForcing(ForceStatus::FORCED);
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
