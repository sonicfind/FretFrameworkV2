#include "InstrumentalTrack_DrumsLegacy.h"

template <>
void InstrumentalTrack<DrumNote_Legacy>::parseText(Midi_Tracker<DrumNote_Legacy>& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		tracker.ext.enableDynamics = true;
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}
