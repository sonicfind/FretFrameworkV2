#pragma once
#include "DrumScan.h"
#include "Extended_Scan.h"

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<4, true>>::testSingleNote(ScanValues& values, size_t lane) noexcept;

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<5, false>>::testSingleNote(ScanValues& values, size_t lane) noexcept;

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<4, true>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept;

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<5, false>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept;
