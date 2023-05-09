#include "Extended_Scan_Drums.h"

template<class DrumType, size_t numPads>
bool TestDrumIndex(ScanValues& values, size_t diff, size_t lane)
{
	if (!DrumNote<DrumType, numPads>::TestIndex(lane))
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
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad_Pro, 4>>::testSingleNote(ScanValues& values, size_t lane) noexcept
{
	return TestDrumIndex<DrumPad_Pro, 4>(values, m_difficulty, lane);
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad, 5>>::testSingleNote(ScanValues& values, size_t lane) noexcept
{
	return TestDrumIndex<DrumPad, 5>(values, m_difficulty, lane);
}

template<class DrumType, size_t numPads>
bool TestDrumIndices(ScanValues& values, size_t diff, const std::vector<std::pair<size_t, uint32_t>>& colors)
{
	unsigned char scanValue = 0;
	for (const auto& color : colors)
	{
		if (!DrumNote<DrumType, numPads>::TestIndex(color.first))
			return false;

		unsigned char result = diff == 3 && color.first == 1 ? 24 : 1 << diff;
		if (result > scanValue)
			scanValue = result;
	}

	values.m_subTracks |= scanValue;
	return InstrumentalScan::WasTrackValidated<DrumNote<DrumType, numPads>>(values, diff);
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad_Pro, 4>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept
{
	return TestDrumIndices<DrumPad_Pro, 4>(values, m_difficulty, colors);
}

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad, 5>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept
{
	return TestDrumIndices<DrumPad, 5>(values, m_difficulty, colors);
}
