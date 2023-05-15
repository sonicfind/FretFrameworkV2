#include "Midi_Scan_Guitar.h"

template <>
Midi_Scanner_Instrument::Scanner_Lanes<GuitarNote<5>>::Scanner_Lanes() : values{
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
} {}

template <>
void Midi_Scanner_Instrument::Scanner<GuitarNote<5>>::parseText(std::string_view str)
{
	if (str == "[ENHANCED_OPENS]" || str == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			m_lanes.values[12 * diff] = 0;
	}
}
