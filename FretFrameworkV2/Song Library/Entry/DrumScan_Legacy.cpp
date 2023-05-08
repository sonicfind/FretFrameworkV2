#include "DrumScan_Legacy.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}

DrumScan_Legacy::DrumScan_Legacy(DrumType_Enum type) : m_type(type) {}
