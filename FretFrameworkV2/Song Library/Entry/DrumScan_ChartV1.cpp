#include "DrumScan_ChartV1.h"

template<size_t numPads, bool PRO_DRUMS>
bool Test_(ScanValues& values, size_t diff, size_t note)
{
	if (diff < 3)
	{
		if (DrumNote<numPads, PRO_DRUMS>::TestIndex_V1(note))
		{
			values.addSubTrack(diff);
			return true;
		}
		return false;
	}
	else
	{
		if (note == 32)
			values.addSubTrack(4);
		else if (DrumNote<numPads, PRO_DRUMS>::TestIndex_V1(note))
			values.addSubTrack(3);
		return values.m_subTracks >= 24;
	}
}

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<4, true>>::test(ScanValues& values, size_t note) const noexcept
{
	return Test_<4, true>(values, m_difficulty, note);
}

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<5, false>>::test(ScanValues& values, size_t note) const noexcept
{
	return Test_<5, false>(values, m_difficulty, note);
}
