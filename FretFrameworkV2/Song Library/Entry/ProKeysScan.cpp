#include "ProKeysScan.h"

template <>
bool InstrumentScan<Keys_Pro>::isComplete() const noexcept { return m_subTracks == 1; }
