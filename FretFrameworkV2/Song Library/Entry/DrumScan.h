#pragma once
#include "InstrumentScan.h"
#include "Notes/DrumNote.h"

template <class DrumType, size_t numPads>
class InstrumentScan<DrumNote<DrumType, numPads>> : public ScanValues
{
public:
	void addDifficulty(size_t diff) noexcept
	{
		m_subTracks |= 1 << diff;
	}

	[[nodiscard]] bool hasSubTrack(size_t diff)
	{
		if (diff < 3)
			return ScanValues::hasSubTrack(diff);
		else
			return m_subTracks >= 24;
	}

	bool isComplete() const noexcept { return m_subTracks == 31; }
	void setIntensity(unsigned char level) noexcept { m_intensity = level; }

	template <class Type, size_t numDrumPads>
	void transfer(InstrumentScan<DrumNote<Type, numDrumPads>>& scan)
	{
		for (size_t i = 0; i < 5; ++i)
			if (hasSubTrack(i))
				scan.addDifficulty(i);
	}

	void reset() noexcept { m_subTracks = 0; }
};
