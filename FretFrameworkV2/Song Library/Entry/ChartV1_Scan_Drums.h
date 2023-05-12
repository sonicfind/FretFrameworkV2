#pragma once
#include "DrumScan_Legacy.h"
#include "ChartV1_Scan.h"

template <>
bool ChartV1_Scan::Test<DrumNote<DrumPad_Pro, 4>>(InstrumentScan<DrumNote<DrumPad_Pro, 4>>& scan, size_t diff, size_t note);

template <>
bool ChartV1_Scan::Test<DrumNote<DrumPad, 5>>(InstrumentScan<DrumNote<DrumPad, 5>>& scan, size_t diff, size_t note);

template <>
bool ChartV1_Scan::Test<DrumNote_Legacy>(InstrumentScan<DrumNote_Legacy>& scan, size_t diff, size_t note);
