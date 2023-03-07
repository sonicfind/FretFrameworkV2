#include "VocalPercussion.h"

bool VocalPercussion::modify(char modifier)
{
	switch (modifier)
	{
	case 'N':
		togglePlayable();
		return true;
	default:
		return false;
	}
}

bool VocalPercussion::isPlayable() const
{
	return m_isPlayable;
}

void VocalPercussion::togglePlayable()
{
	m_isPlayable = !m_isPlayable;
}

void VocalPercussion::setPlayable(bool enable)
{
	m_isPlayable = enable;
}

std::vector<char> VocalPercussion::getActiveModifiers() const
{
	std::vector<char> mods;
	if (!m_isPlayable)
		mods.push_back('N');
	return mods;
}
