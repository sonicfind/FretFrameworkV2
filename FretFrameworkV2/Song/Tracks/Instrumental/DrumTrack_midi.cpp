#include "DrumTrack_midi.h"

template <>
void InstrumentalTrack<DrumNote<4, true>, true>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		tracker.ext.enableDynamics = true;
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}

template <>
void InstrumentalTrack<DrumNote<5, false>, true>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		tracker.ext.enableDynamics = true;
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}

template <>
void InstrumentalTrack<DrumNote<4, true>, true>::writeMidiToggleEvent(MidiFileWriter& writer) const
{
	writer.addText(0, "[ENABLE_CHART_DYNAMICS]");
}

template <>
void InstrumentalTrack<DrumNote<5, false>, true>::writeMidiToggleEvent(MidiFileWriter& writer) const
{
	writer.addText(0, "[ENABLE_CHART_DYNAMICS]");
}
