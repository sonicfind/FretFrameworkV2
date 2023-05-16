#pragma once
#include "InstrumentScan.h"
#include "Notes/DrumNote.h"

template<>
bool InstrumentScan<DrumNote<DrumPad_Pro, 4>>::isComplete() const noexcept;

template<>
bool InstrumentScan<DrumNote<DrumPad, 5>>::isComplete() const noexcept;
