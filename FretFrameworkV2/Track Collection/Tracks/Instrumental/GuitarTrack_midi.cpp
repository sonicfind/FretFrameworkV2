#include "GuitarTrack_midi.h"

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];

	if (lane == -1)
		return;

	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 6)
	{
		if (m_inOnState)
		{
			m_tracker.difficulties[diff].notes[lane] = position;

			if (GuitarNote<5>*guitar = m_track.construct_note_midi(diff, position))
			{
				if (m_tracker.difficulties[diff].sliderNotes)
					guitar->setTap(true);

				if (m_tracker.difficulties[diff].hopoOn)
					guitar->setForcing(ForceStatus::HOPO_ON);
				else if (m_tracker.difficulties[diff].hopoOff)
					guitar->setForcing(ForceStatus::HOPO_OFF);
			}
		}
		else
		{
			uint32_t& colorPosition = m_tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.getNote_midi(diff, colorPosition).set(lane, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
	// HopoON marker
	else if (lane == 6)
	{
		m_tracker.difficulties[diff].hopoOn = m_inOnState;
		if (m_inOnState)
		{
			if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(diff, position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 7)
	{
		m_tracker.difficulties[diff].hopoOff = m_inOnState;
		if (m_inOnState)
		{
			if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(diff, position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
	else if (lane == 8)
	{
		if (diff == 3)
		{
			addSpecialPhrase(m_solo);
			return;
		}

		m_track.convertSoloesToStarPower_midi();
		addSpecialPhrase(diff, m_tracker.difficulties[diff].starPower);

		for (size_t i = 0; i < 4; ++i)
			m_laneValues[12 * i + 8] = 12;
	}
	else if (lane == 9)
		m_tracker.difficulties[diff].sliderNotes = m_inOnState;
	else if (lane == 10)
		addSpecialPhrase(diff, m_tracker.difficulties[diff].faceOff[0]);
	else if (lane == 11)
		addSpecialPhrase(diff, m_tracker.difficulties[diff].faceOff[1]);
	else if (lane == 12)
		addSpecialPhrase(diff, m_tracker.difficulties[diff].starPower);
}

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 7)
	{
		if (m_inOnState)
		{
			m_tracker.difficulties[diff].notes[lane] = position;

			if (GuitarNote<6>*guitar = m_track.construct_note_midi(diff, position))
			{
				if (m_tracker.difficulties[diff].sliderNotes)
					guitar->setTap(true);

				if (m_tracker.difficulties[diff].hopoOn)
					guitar->setForcing(ForceStatus::HOPO_ON);
				else if (m_tracker.difficulties[diff].hopoOff)
					guitar->setForcing(ForceStatus::HOPO_OFF);
			}
		}
		else
		{
			uint32_t& colorPosition = m_tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.getNote_midi(diff, colorPosition).set(lane, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
	// HopoON marker
	else if (lane == 7)
	{
		m_tracker.difficulties[diff].hopoOn = m_inOnState;
		if (m_inOnState)
		{
			if (GuitarNote<6>*note = m_track.testBackNote_midiOnly(diff, position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 8)
	{
		m_tracker.difficulties[diff].hopoOff = m_inOnState;
		if (m_inOnState)
		{
			if (GuitarNote<6>*note = m_track.testBackNote_midiOnly(diff, position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
}

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
							note->setTap(true);
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
				if (sysex[6])
				{
					m_tracker.difficulties[sysex[4]].sliderNotes = true;
					if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(sysex[4], position))
						note->setTap(true);
				}
				else
					m_tracker.difficulties[sysex[4]].sliderNotes = false;
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
		m_track.get_or_emplace_Events_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
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
	if (sysex.compare(0, 2, "PS") == 0 && sysex[5] == 4)
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
						note->setTap(true);
				}
			}
		}
		else if (sysex[6])
		{
			m_tracker.difficulties[sysex[4]].sliderNotes = true;
			if (GuitarNote<6>*note = m_track.testBackNote_midiOnly(sysex[4], position))
				note->setTap(true);
		}
		else
			m_tracker.difficulties[sysex[4]].sliderNotes = false;
	}
}
