#include "Extended_Scan_Drums.h"

template<class DrumType, size_t numPads>
bool TestDrumIndex(InstrumentScan<DrumNote<DrumType, numPads>>& scan, size_t diff, size_t lane)
{
	if (!DrumNote<DrumType, numPads>::TestIndex(lane))
		return false;

	scan.addDifficulty(diff);
	if (diff == 3)
	{
		if (lane != 1)
			return false;

		scan.addDifficulty(4);
	}
	return true;
}

template<>
bool Extended_Scan::TestSingleNote<DrumNote<DrumPad_Pro, 4>>(InstrumentScan<DrumNote<DrumPad_Pro, 4>>& scan, size_t diff, size_t lane) noexcept
{
	return TestDrumIndex(scan, diff, lane);
}

template<>
bool Extended_Scan::TestSingleNote<DrumNote<DrumPad, 5>>(InstrumentScan<DrumNote<DrumPad, 5>>& scan, size_t diff, size_t lane) noexcept
{
	return TestDrumIndex(scan, diff, lane);
}

template<class DrumType, size_t numPads>
bool TestDrumIndices(InstrumentScan<DrumNote<DrumType, numPads>>& scan, size_t diff, const std::vector<std::pair<size_t, uint64_t>>& colors)
{
	unsigned char scanValue = 0;
	bool doubleBass = false;
	for (const auto& color : colors)
	{
		if (!DrumNote<DrumType, numPads>::TestIndex(color.first))
			return false;

		if (diff == 3 && color.first == 1)
			doubleBass = true;
	}

	scan.addDifficulty(diff);
	if (doubleBass)
		scan.addDifficulty(4);
	return diff < 3 || doubleBass;
}

template<>
bool Extended_Scan::TestMultiNote<DrumNote<DrumPad_Pro, 4>>(InstrumentScan<DrumNote<DrumPad_Pro, 4>>& scan, size_t diff, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept
{
	return TestDrumIndices(scan, diff, colors);
}

template<>
bool Extended_Scan::TestMultiNote<DrumNote<DrumPad, 5>>(InstrumentScan<DrumNote<DrumPad, 5>>& scan, size_t diff, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept
{
	return TestDrumIndices(scan, diff, colors);
}
