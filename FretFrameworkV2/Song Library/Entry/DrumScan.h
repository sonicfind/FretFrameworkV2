#pragma once
#include "InstrumentalScan.h"
#include "Notes/DrumNote.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<4, true>>(const ScanValues& values, size_t diff);

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<5, false>>(const ScanValues& values, size_t diff);
