#pragma once
#include "InstrumentScan.h"
#include "Notes/DrumNote.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<DrumPad_Pro, 4>>(const ScanValues& values, size_t diff);

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<DrumPad, 5>>(const ScanValues& values, size_t diff);
