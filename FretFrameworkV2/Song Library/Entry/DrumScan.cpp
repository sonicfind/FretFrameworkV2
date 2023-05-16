#include "DrumScan.h"

template<>
bool InstrumentScan<DrumNote<DrumPad_Pro, 4>>::isComplete() const noexcept { return m_subTracks == 31; }

template<>
bool InstrumentScan<DrumNote<DrumPad, 5>>::isComplete() const noexcept { return m_subTracks == 31; }
