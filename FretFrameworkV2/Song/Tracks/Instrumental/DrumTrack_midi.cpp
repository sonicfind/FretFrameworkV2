#include "DrumTrack_midi.h"

template <>
void InstrumentalTrack<DrumNote<4, true>>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		tracker.ext.enableDynamics = true;
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}

template <>
void InstrumentalTrack<DrumNote<5, false>>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		tracker.ext.enableDynamics = true;
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}
