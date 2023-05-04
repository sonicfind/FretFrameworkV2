#pragma once
#include <stdint.h>
static constexpr uint32_t BPM_FACTOR = 60000000;
static constexpr uint32_t DEFAULT_BPM = 120;
static constexpr uint32_t MICROS_AT_120BPM = BPM_FACTOR / DEFAULT_BPM;

class MicrosPerQuarter
{
	uint32_t m_micros = MICROS_AT_120BPM;

public:
	MicrosPerQuarter() = default;
	MicrosPerQuarter(uint32_t micros) : m_micros(micros) {}
	operator uint32_t() const noexcept { return m_micros; }
	MicrosPerQuarter& operator=(uint32_t micros)
	{
		m_micros = micros;
		return *this;
	}

	void setBPM(float bpm)
	{
		if (bpm == 0)
			m_micros = 0;
		else
			m_micros = uint32_t(BPM_FACTOR / bpm);
	}

	float getBPM() const noexcept
	{
		if (m_micros == 0)
			return 0;
		return (float)BPM_FACTOR / m_micros;
	}
};

