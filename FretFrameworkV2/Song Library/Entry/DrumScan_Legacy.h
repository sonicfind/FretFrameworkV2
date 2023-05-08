#pragma once
#include "DrumScan.h"
#include "Notes/DrumNote_Legacy.h"

class DrumScan_Legacy
{
protected:
	DrumType_Enum m_type;

public:
	DrumScan_Legacy(DrumType_Enum type);
	[[nodiscard]] DrumType_Enum getDrumType() const noexcept { return m_type; }
};

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(const ScanValues& values, size_t diff);
