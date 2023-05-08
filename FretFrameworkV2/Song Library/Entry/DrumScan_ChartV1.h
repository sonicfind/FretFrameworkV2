#pragma once
#include "DrumScan.h"
#include "Notes/DrumNote_Legacy.h"
#include "InstrumentalScan_ChartV1.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(const ScanValues& values, size_t diff);

template<>
struct InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy>
{
	DrumType_Enum m_type;
	size_t m_difficulty = 0;

	V1Tracker(DrumType_Enum type);
	[[nodiscard]] DrumType_Enum getDrumType() const noexcept { return m_type; }
	[[nodiscard]] bool setDifficulty(ScanValues& values, size_t diff);
	[[nodiscard]] bool test(ScanValues& values, size_t note);
};

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<4, true>>::test(ScanValues& values, size_t note) const noexcept;

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<5, false>>::test(ScanValues& values, size_t note) const noexcept;
