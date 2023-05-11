#include "Midi_Loader_Drums.h"

template <>
void Midi_Loader<DrumNote<DrumPad_Pro, 4>>::modNote(DrumNote<DrumPad_Pro, 4>& note, size_t diff, size_t lane, unsigned char velocity)
{
	if (m_difficulties[diff].flam)
		note.setFlam(true);

	if (lane > 0)
	{
		auto& pad = note.get(lane - 1);
		if (2 <= lane && lane < 5)
			pad.setCymbal(!m_ext.toms[lane - 2]);

		if (m_ext.enableDynamics)
		{
			if (velocity > 100)
				pad.setDynamics(DrumDynamics::Accent);
			else if (velocity < 100)
				pad.setDynamics(DrumDynamics::Ghost);
		}
	}
}

template <>
void Midi_Loader<DrumNote<DrumPad, 5>>::modNote(DrumNote<DrumPad, 5>& note, size_t diff, size_t lane, unsigned char velocity)
{
	if (m_ext.enableDynamics && lane > 0)
	{
		if (velocity > 100)
			note.get(lane - 1).setDynamics(DrumDynamics::Accent);
		else if (velocity < 100)
			note.get(lane - 1).setDynamics(DrumDynamics::Ghost);
	}
}

template <>
void Midi_Loader<DrumNote_Legacy>::modNote(DrumNote_Legacy& note, size_t diff, size_t lane, unsigned char velocity)
{
	if (m_difficulties[diff].flam)
		note.setFlam(true);

	if (lane > 0)
	{
		auto& pad = note.get(lane - 1);
		if (lane == 5)
		{
			if (m_ext.type == DrumType_Enum::LEGACY)
				m_ext.type = DrumType_Enum::FIVELANE;
		}
		else if (2 <= lane && m_ext.type != DrumType_Enum::FIVELANE)
			pad.setCymbal(!m_ext.toms[lane - 2]);

		if (m_ext.enableDynamics)
		{
			if (velocity > 100)
				pad.setDynamics(DrumDynamics::Accent);
			else if (velocity < 100)
				pad.setDynamics(DrumDynamics::Ghost);
		}
	}
}

template <>
void Midi_Loader<DrumNote<DrumPad_Pro, 4>>::parseText(std::string_view str)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		m_ext.enableDynamics = true;
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}

template <>
void Midi_Loader<DrumNote<DrumPad, 5>>::parseText(std::string_view str)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		m_ext.enableDynamics = true;
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}

template <>
void Midi_Loader<DrumNote_Legacy>::parseText(std::string_view str)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		m_ext.enableDynamics = true;
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}
