#include "GuitarScan_midi.h"

template <>
void InstrumentalScan<GuitarNote<5>>::parseText(Midi_Scanner& tracker, std::string_view str)
{
	if (str == "[ENHANCED_OPENS]" || str == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			tracker.laneValues[12 * diff] = 0;
	}
}
