#include "DrumTrack_midi.h"

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::init()
{
	m_noteRange = { 60, 102 };
}

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::init()
{
	m_noteRange = { 60, 102 };
}

template <>
void InstrumentTrackMidiParser<DrumNote<4, true>>::parseText(std::string_view text)
{
	if (text == "[ENABLE_CHART_DYNAMICS]")
		m_tracker.enableDynamics = true;
	else
		m_track.getEvents_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32( text));
}

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::parseText(std::string_view text)
{
	if (text == "[ENABLE_CHART_DYNAMICS]")
		m_tracker.enableDynamics = true;
	else
		m_track.getEvents_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
}

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseText(std::string_view text)
{
	if (text == "[ENABLE_CHART_DYNAMICS]")
		m_tracker.enableDynamics = true;
	else
		m_track.getEvents_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
}
