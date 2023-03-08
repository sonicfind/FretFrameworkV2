#include "GuitarTrack_midi.h"

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::init()
{
	static constexpr int FIVEFRET_LANES[48] =
	{
		-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	};

	memcpy(m_laneValues, FIVEFRET_LANES, sizeof(FIVEFRET_LANES));
	m_noteRange = { 59, 108 };
}

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseSysEx(std::string_view sysex)
{
	if (sysex.compare(0, 2, "PS") == 0)
	{
		const uint32_t position = m_reader.getPosition();
		if (sysex[4] == 0xFF)
		{
			switch (sysex[5])
			{
			case 1:
				for (size_t diff = 0; diff < 4; ++diff)
					m_laneValues[12 * diff + 1] = !sysex[6];
				break;
			case 4:
				for (size_t diff = 0; diff < 4; ++diff)
				{
					m_tracker.difficulties[diff].sliderNotes = sysex[6];
					if (sysex[6])
					{
						if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(diff, position))
							note->modify('T');
					}
				}
				break;
			}
		}
		else
		{
			switch (sysex[5])
			{
			case 1:
				m_laneValues[12 * sysex[4] + 1] = !sysex[6];
				break;
			case 4:
				m_tracker.difficulties[sysex[4]].sliderNotes = sysex[6];
				if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(sysex[4], position))
					note->modify('T');
				break;
			}
		}
	}
}

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseText(std::string_view text)
{
	if (text == "[ENHANCED_OPENS]" || text == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			m_laneValues[12 * diff] = 0;
	}
	else
		m_track.getEvents_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
}

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::init()
{
	static constexpr int SIXFRET_LANES[48] =
	{
		// White and black values are swapped in the file for some reason
		0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
		0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
		0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
		0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	};

	memcpy(m_laneValues, SIXFRET_LANES, sizeof(SIXFRET_LANES));
	m_noteRange = { 58, 103 };
}

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::parseSysEx(std::string_view sysex)
{
	if (sysex.compare(0, 2, "PS") == 0)
	{
		if (sysex[5] == 4)
		{
			const uint32_t position = m_reader.getPosition();
			if (sysex[4] == 0xFF)
			{
				for (size_t diff = 0; diff < 4; ++diff)
				{
					m_tracker.difficulties[diff].sliderNotes = sysex[6];
					if (sysex[6])
					{
						if (GuitarNote<6>* note = m_track.testBackNote_midiOnly(diff, position))
							note->modify('T');
					}
				}
			}
			else
			{
				m_tracker.difficulties[sysex[4]].sliderNotes = sysex[6];
				if (GuitarNote<6>*note = m_track.testBackNote_midiOnly(sysex[4], position))
					note->modify('T');
			}
		}
	}
}
