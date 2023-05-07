#pragma once
#include "DrumScan.h"
#include "InstrumentalScan_ChartV1.h"

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<4, true>>::test(ScanValues& values, size_t note) const noexcept;

template <>
bool InstrumentalScan_ChartV1::V1Tracker<DrumNote<5, false>>::test(ScanValues& values, size_t note) const noexcept;
