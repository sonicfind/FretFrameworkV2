#include "GuitarTrack_midi.h"

template <>
void InstrumentalTrack<GuitarNote<5>>::parseSysEx(Midi_Tracker<GuitarNote<5>>& tracker, std::string_view str, uint32_t position)
{
	if (str.compare(0, 2, "PS") == 0)
	{
		if (str[4] == (char)0xFF)
		{
			switch (str[5])
			{
			case 1:
				for (size_t diff = 0; diff < 4; ++diff)
					tracker.laneValues[12 * diff + 1] = !str[6];
				break;
			case 4:
				for (size_t diff = 0; diff < 4; ++diff)
				{
					tracker.difficulties[diff].sliderNotes = str[6];
					if (str[6])
					{
						if (GuitarNote<5>*note = m_difficulties[diff].m_notes.try_back(position))
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
				tracker.laneValues[12 * str[4] + 1] = !str[6];
				break;
			case 4:
				if (str[6])
				{
					tracker.difficulties[str[4]].sliderNotes = true;
					if (GuitarNote<5>*note = m_difficulties[str[4]].m_notes.try_back(position))
						note->setTap(true);
				}
				else
					tracker.difficulties[str[4]].sliderNotes = false;
				break;
			}
		}
	}
}

template <>
void InstrumentalTrack<GuitarNote<6>>::parseSysEx(Midi_Tracker<GuitarNote<6>>& tracker, std::string_view str, uint32_t position)
{
	if (str.compare(0, 2, "PS") == 0 && str[5] == 4)
	{
		if (str[4] == (char)0xFF)
		{
			for (size_t diff = 0; diff < 4; ++diff)
			{
				tracker.difficulties[diff].sliderNotes = str[6];
				if (str[6])
				{
					if (GuitarNote<6>*note = m_difficulties[diff].m_notes.try_back(position))
						note->setTap(true);
				}
			}
		}
		else if (str[6])
		{
			tracker.difficulties[str[4]].sliderNotes = true;
			if (GuitarNote<6>*note = m_difficulties[str[4]].m_notes.try_back(position))
				note->setTap(true);
		}
		else
			tracker.difficulties[str[4]].sliderNotes = false;
	}
}

template <>
void InstrumentalTrack<GuitarNote<5>>::parseText(Midi_Tracker<GuitarNote<5>>& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENHANCED_OPENS]" || str == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			tracker.laneValues[12 * diff] = 0;
	}
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}
