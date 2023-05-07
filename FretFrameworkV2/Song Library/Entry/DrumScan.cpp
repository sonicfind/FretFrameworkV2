#include "DrumScan.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<4, true>>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<5, false>>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}
