#pragma once
#include <stdint.h>

class NoteColor
{
	uint16_t m_sustain = 0;

public:
	uint32_t getSustain() const { return m_sustain; }
	void set(uint32_t sustain)
	{
		if (sustain == 0)
			sustain = 1;
		m_sustain = sustain;
	}
	void disable() { m_sustain = 0; }
	bool isActive() const { return m_sustain > 0; }

	void operator*=(float multiplier)
	{
		if (m_sustain)
		{
			float sustain = m_sustain * multiplier;
			if (sustain < 1)
				sustain = 1;
			m_sustain = uint32_t(sustain);
		}
	}
};
