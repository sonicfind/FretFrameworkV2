#pragma once
#include "DrumScan.h"
#include "Extended_Scan.h"

template<>
bool Extended_Scan::TestSingleNote<DrumNote<DrumPad_Pro, 4>>(InstrumentScan<DrumNote<DrumPad_Pro, 4>>& scan, size_t diff, size_t lane) noexcept;

template<>
bool Extended_Scan::TestSingleNote<DrumNote<DrumPad, 5>>(InstrumentScan<DrumNote<DrumPad, 5>>& scan, size_t diff, size_t lane) noexcept;

template<>
bool Extended_Scan::TestMultiNote<DrumNote<DrumPad_Pro, 4>>(InstrumentScan<DrumNote<DrumPad_Pro, 4>>& scan, size_t diff, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept;

template<>
bool Extended_Scan::TestMultiNote<DrumNote<DrumPad, 5>>(InstrumentScan<DrumNote<DrumPad, 5>>& scan, size_t diff, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept;
