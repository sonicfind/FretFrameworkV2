#pragma once
#include <stdint.h>

class Sustained
{
public:
	static uint32_t s_minLength;

protected:
	uint32_t m_length = 0;

public:
	constexpr void setLength(uint32_t length)
	{
		if (length < s_minLength)
			length = 1;
		m_length = length;
	}
	void disable() { m_length = 0; }

	uint32_t getLength() const { return m_length; }
	bool isActive() const { return m_length > 0; }

	void operator*=(float multiplier)
	{
		if (m_length)
		{
			float sustain = m_length * multiplier;
			if (sustain < 1)
				sustain = 1;
			m_length = uint32_t(sustain);
		}
	}
};
