#include "Midi_Loader_Guitar.h"

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote<5>>::Loader_Lanes() : values{
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
}{}

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote<6>>::Loader_Lanes() : values{
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
} {}

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::modNote(GuitarNote<5>& note, size_t diff, size_t lane, unsigned char velocity)
{
	if (m_difficulties[diff].sliderNotes)
		note.setTap(true);

	if (m_difficulties[diff].hopoOn)
		note.setForcing(ForceStatus::HOPO_ON);
	else if (m_difficulties[diff].hopoOff)
		note.setForcing(ForceStatus::HOPO_OFF);
}

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<6>>::modNote(GuitarNote<6>& note, size_t diff, size_t lane, unsigned char velocity)
{
	if (m_difficulties[diff].sliderNotes)
		note.setTap(true);

	if (m_difficulties[diff].hopoOn)
		note.setForcing(ForceStatus::HOPO_ON);
	else if (m_difficulties[diff].hopoOff)
		note.setForcing(ForceStatus::HOPO_OFF);
}

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::parseSysEx(std::string_view str)
{
	if (str.compare(0, 2, "PS") == 0)
	{
		if (str[6])
		{
			if (m_position <= m_lastOn + 16)
				m_position = m_lastOn;
			else
				m_lastOn = m_position;
		}

		if (str[4] == (char)0xFF)
		{
			switch (str[5])
			{
			case 1:
				for (size_t diff = 0; diff < 4; ++diff)
					m_lanes.values[12 * diff + 1] = !str[6];
				break;
			case 4:
				for (size_t diff = 0; diff < 4; ++diff)
				{
					m_difficulties[diff].sliderNotes = str[6];
					if (str[6])
					{
						if (auto note = m_track[diff].m_notes.try_back(m_position))
							note->setTap(true);
					}
				}
				break;
			}
		}
		else
		{
			switch (str[5])
			{
			case 1:
				m_lanes.values[12 * str[4] + 1] = !str[6];
				break;
			case 4:
				if (str[6])
				{
					m_difficulties[str[4]].sliderNotes = true;
					if (auto note = m_track[str[4]].m_notes.try_back(m_position))
						note->setTap(true);
				}
				else
					m_difficulties[str[4]].sliderNotes = false;
				break;
			}
		}
	}
}

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<6>>::parseSysEx(std::string_view str)
{
	if (str.compare(0, 2, "PS") == 0 && str[5] == 4)
	{
		if (str[6])
		{
			if (m_position <= m_lastOn + 16)
				m_position = m_lastOn;
			else
				m_lastOn = m_position;

			if (str[4] == (char)0xFF)
			{
				for (size_t diff = 0; diff < 4; ++diff)
				{
					m_difficulties[diff].sliderNotes = true;
					if (auto note = m_track[diff].m_notes.try_back(m_position))
						note->setTap(true);
				}
			}
			else
			{
				m_difficulties[str[4]].sliderNotes = true;
				if (auto note = m_track[str[4]].m_notes.try_back(m_position))
					note->setTap(true);
			}
		}
		else if (str[4] == (char)0xFF)
		{
			for (size_t diff = 0; diff < 4; ++diff)
				m_difficulties[diff].sliderNotes = false;
		}
		else
			m_difficulties[str[4]].sliderNotes = false;
	}
}

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::parseText(std::string_view str)
{
	if (str == "[ENHANCED_OPENS]" || str == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			m_lanes.values[12 * diff] = 0;
	}
	else
		m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
}
