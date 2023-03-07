#pragma once
#include <stdint.h>
#include "../File Processing/TimeSig.h"

class SyncValues
{
	uint32_t m_microsPerQuarter = 0;
	TimeSig m_timeSig;

	// Microseconds
	uint32_t m_anchorPoint = 0;

public:
	void setMicrosPerQuarter(uint32_t micros);
	void setMicrosPerQuarter(float bpm);
	void setTimeSig(TimeSig timeSig);
	void setAnchor(uint32_t anchor);

	uint32_t getMicros() const noexcept { return m_microsPerQuarter; }
	float getBPM() const noexcept { return (float)BPM_FACTOR / m_microsPerQuarter; }
	TimeSig getTimeSig() const noexcept { return m_timeSig; }
	uint32_t getAnchor() const noexcept { return m_anchorPoint; }
};

