#include "DrumScan.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<DrumPad_Pro, 4>>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote<DrumPad, 5>>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}
