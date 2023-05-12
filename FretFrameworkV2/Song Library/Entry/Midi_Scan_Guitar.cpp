#include "Midi_Scan_Guitar.h"

template <>
void Midi_Scanner_Instrument::Scanner<GuitarNote<5>>::parseText(std::string_view str)
{
	if (str == "[ENHANCED_OPENS]" || str == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			m_laneValues[12 * diff] = 0;
	}
}
