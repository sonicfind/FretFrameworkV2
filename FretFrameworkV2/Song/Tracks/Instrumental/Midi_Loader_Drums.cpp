#include "Midi_Loader_Drums.h"

template <class DrumType, size_t numPads>
void Mod(Midi_Loader_Instrument::Loader_Ext<DrumNote<DrumType, numPads>>& ext,DrumNote<DrumType, numPads>& note, size_t diff, size_t lane, unsigned char velocity)
{
	if (ext.flams[diff])
		note.setFlam(true);

	if (lane > 0)
	{
		auto& pad = note.get(lane - 1);
		if constexpr (std::is_same<DrumType, DrumPad_Pro>::value)
			if (2 <= lane && lane < 5)
				pad.setCymbal(!ext.toms[lane - 2]);

		if (ext.enableDynamics)
		{
			if (velocity > 100)
				pad.setDynamics(DrumDynamics::Accent);
			else if (velocity < 100)
				pad.setDynamics(DrumDynamics::Ghost);
		}
	}
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad_Pro, 4>>::modNote(DrumNote<DrumPad_Pro, 4>& note, size_t diff, size_t lane, unsigned char velocity)
{
	Mod(m_ext, note, diff, lane, velocity);
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::modNote(DrumNote<DrumPad, 5>& note, size_t diff, size_t lane, unsigned char velocity)
{
	Mod(m_ext, note, diff, lane, velocity);
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote_Legacy>::modNote(DrumNote_Legacy& note, size_t diff, size_t lane, unsigned char velocity)
{
	Mod(m_ext, note, diff, lane, velocity);
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad_Pro, 4>>::parseText(std::string_view str)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		m_ext.enableDynamics = true;
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::parseText(std::string_view str)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		m_ext.enableDynamics = true;
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote_Legacy>::parseText(std::string_view str)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		m_ext.enableDynamics = true;
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}
