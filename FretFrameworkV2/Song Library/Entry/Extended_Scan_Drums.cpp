#include "Extended_Scan_Drums.h"

template<size_t numPads, bool PRO_DRUMS>
bool TestDrumIndex(ScanValues& values, size_t diff, size_t lane)
{
	if (!DrumNote<numPads, PRO_DRUMS>::TestIndex(lane))
		return false;

	values.addSubTrack(diff);
	if (diff == 3)
	{
		if (lane != 1)
			return false;

		values.addSubTrack(4);
	}
	return true;
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<4, true>>::testSingleNote(ScanValues& values, size_t lane) noexcept
{
	return TestDrumIndex<4, true>(values, m_difficulty, lane);
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<5, false>>::testSingleNote(ScanValues& values, size_t lane) noexcept
{
	return TestDrumIndex<5, false>(values, m_difficulty, lane);
}

template<size_t numPads, bool PRO_DRUMS>
bool TestDrumIndices(ScanValues& values, size_t diff, const std::vector<std::pair<size_t, uint32_t>>& colors)
{
	unsigned char scanValue = 0;
	for (const auto& color : colors)
	{
		if (!DrumNote<numPads, PRO_DRUMS>::TestIndex(color.first))
			return false;

		unsigned char result = diff == 3 && color.first == 1 ? 24 : 1 << diff;
		if (result > scanValue)
			scanValue = result;
	}

	values.m_subTracks |= scanValue;
	return InstrumentalScan::WasTrackValidated<DrumNote<numPads, PRO_DRUMS>>(values, diff);
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<4, true>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept
{
	return TestDrumIndices<4, true>(values, m_difficulty, colors);
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<5, false>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept
{
	return TestDrumIndices<5, false>(values, m_difficulty, colors);
}
