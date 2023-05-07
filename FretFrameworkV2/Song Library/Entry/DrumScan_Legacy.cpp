#include "DrumScan_Legacy.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}

void DrumScan_Legacy::transfer(ScanValues& values) const noexcept
{
	values.m_subTracks |= m_values.m_subTracks;
}
