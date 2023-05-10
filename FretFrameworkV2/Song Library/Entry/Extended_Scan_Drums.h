#pragma once
#include "DrumScan.h"
#include "Extended_Scan.h"

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad_Pro, 4>>::testSingleNote(ScanValues& values, size_t lane) noexcept;

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad, 5>>::testSingleNote(ScanValues& values, size_t lane) noexcept;

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad_Pro, 4>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept;

template<>
bool Extended_Scan::DifficultyTracker<DrumNote<DrumPad, 5>>::testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept;
