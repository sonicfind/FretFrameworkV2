#pragma once
#include "DrumScan_Legacy.h"
#include "InstrumentalScan_ChartV1.h"

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<4, true>>::test(ScanValues& values, size_t note) const noexcept;

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<5, false>>::test(ScanValues& values, size_t note) const noexcept;

template<>
struct InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy> : public DrumScan_Legacy
{
	size_t m_difficulty = 0;

	V1Tracker(DrumType_Enum type);
	[[nodiscard]] bool setDifficulty(ScanValues& values, size_t diff);
	[[nodiscard]] bool test(ScanValues& values, size_t note);
};
