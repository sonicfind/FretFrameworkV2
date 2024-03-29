#pragma once
#include "ScanValues.h"

template <class T>
class InstrumentScan : public ScanValues
{
public:
	void addDifficulty(size_t diff) noexcept
	{
		m_subTracks |= 1 << diff;
	}

	bool isComplete() const noexcept { return m_subTracks == 15; }
	void setIntensity(unsigned char level) noexcept { m_intensity = level; }
	void reset() { m_subTracks = 0; }

	template <class U>
	InstrumentScan<T>& operator=(InstrumentScan<U>& scan)
	{
		ScanValues::operator=(scan);
		return *this;
	}
};
