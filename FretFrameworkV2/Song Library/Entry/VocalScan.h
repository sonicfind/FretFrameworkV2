#pragma once
#include "ScanValues.h"

template <size_t numTracks>
class VocalScan : public ScanValues
{
public:
	void addSubTrack(size_t diff) noexcept
	{
		m_subTracks |= 1 << diff;
	}

	bool isComplete() const noexcept { return m_subTracks == (1 << numTracks) - 1; }
	void setIntensity(unsigned char level) noexcept { m_intensity = level; }
};
