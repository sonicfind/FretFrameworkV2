#include "DrumScan_Legacy.h"
InstrumentScan<DrumNote_Legacy>::InstrumentScan(DrumType_Enum type) : m_type(type) {}

void InstrumentScan<DrumNote_Legacy>::setType(DrumType_Enum type)
{
	if (m_type == DrumType_Enum::LEGACY)
		m_type = type;
}
