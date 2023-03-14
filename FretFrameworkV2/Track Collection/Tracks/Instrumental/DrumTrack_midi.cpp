#include "DrumTrack_midi.h"

template <>
void InstrumentTrackMidiParser<DrumNote<4, true>>::parseLaneColor(MidiNote note, const bool isON)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 5)
	{
		if (isON)
		{
			m_tracker.difficulties[diff].notes[lane] = position;
			DrumNote<4, true>* drums = nullptr;
			if (drums = m_track.construct_note_midi(diff, position))
			{
				if (m_tracker.difficulties[diff].flam)
					drums->setFlam(true);
			}
			else
				drums = &m_track.backNote_midiOnly(diff);

			if (2 <= lane && lane < 5)
				drums->setCymbal(lane, !m_tracker.toms[lane - 2]);

			if (m_tracker.enableDynamics)
			{
				if (note.velocity > 100)
					drums->setDynamics(lane, DrumDynamics::Accent);
				else if (note.velocity < 100)
					drums->setDynamics(lane, DrumDynamics::Ghost);
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
	else if (note.value == 95)
	{
		if (isON)
		{
			m_tracker.difficulties[3].notes[0] = position;

			m_track.construct_note_midi(diff, position);
			m_track.backNote_midiOnly(diff).modify('+');
		}
		else
		{
			uint32_t& colorPosition = m_tracker.difficulties[3].notes[0];
			if (colorPosition != UINT32_MAX)
			{
				m_track.getNote_midi(3, colorPosition).set(0, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
}

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::parseLaneColor(MidiNote note, const bool isON)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 6)
	{
		if (isON)
		{
			m_tracker.difficulties[diff].notes[lane] = position;
			DrumNote<5, false>* drums = nullptr;
			if (drums = m_track.construct_note_midi(diff, position))
			{
				if (m_tracker.difficulties[diff].flam)
					drums->setFlam(true);
			}
			else
				drums = &m_track.backNote_midiOnly(diff);

			if (m_tracker.enableDynamics)
			{
				if (note.velocity > 100)
					drums->setDynamics(lane, DrumDynamics::Accent);
				else if (note.velocity < 100)
					drums->setDynamics(lane, DrumDynamics::Ghost);
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
	else if (note.value == 95)
	{
		if (isON)
		{
			m_tracker.difficulties[3].notes[0] = position;

			m_track.construct_note_midi(diff, position);
			m_track.backNote_midiOnly(diff).modify('+');
		}
		else
		{
			uint32_t& colorPosition = m_tracker.difficulties[3].notes[0];
			if (colorPosition != UINT32_MAX)
			{
				m_track.getNote_midi(3, colorPosition).set(0, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
}

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseLaneColor(MidiNote note, const bool isON)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 6)
	{
		if (isON)
		{
			if (lane == 5 && m_track.getDrumType() == DrumType_Enum::LEGACY)
			{
				m_track.setDrumType(DrumType_Enum::FIVELANE);
				for (auto& tom : m_tracker.toms)
					tom = true;
			}

			m_tracker.difficulties[diff].notes[lane] = position;
			DrumNote_Legacy* drums = nullptr;
			if (drums = m_track.construct_note_midi(diff, position))
			{
				if (m_tracker.difficulties[diff].flam)
					drums->setFlam(true);
			}
			else
				drums = &m_track.backNote_midiOnly(diff);

			if (2 <= lane && lane < 5)
				drums->setCymbal(lane, !m_tracker.toms[lane - 2]);

			if (m_tracker.enableDynamics)
			{
				if (note.velocity > 100)
					drums->setDynamics(lane, DrumDynamics::Accent);
				else if (note.velocity < 100)
					drums->setDynamics(lane, DrumDynamics::Ghost);
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
	else if (note.value == 95)
	{
		if (isON)
		{
			m_tracker.difficulties[3].notes[0] = position;

			m_track.construct_note_midi(diff, position);
			m_track.backNote_midiOnly(diff).modify('+');
		}
		else
		{
			uint32_t& colorPosition = m_tracker.difficulties[3].notes[0];
			if (colorPosition != UINT32_MAX)
			{
				m_track.getNote_midi(3, colorPosition).set(0, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
}

template <>
void InstrumentTrackMidiParser<DrumNote<4, true>>::toggleExtraValues(MidiNote note, const bool isON)
{
	if (110 <= note.value && note.value <= 112)
		m_tracker.toms[note.value - 110] = isON;
}

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::toggleExtraValues(MidiNote note, const bool isON)
{
	if (m_track.getDrumType() != DrumType_Enum::FIVELANE && 110 <= note.value && note.value <= 112)
	{
		m_tracker.toms[note.value - 110] = isON;
		m_track.setDrumType(DrumType_Enum::FOURLANE_PRO);
	}
}

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
		m_track.get_or_emplace_Events_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32( text));
}

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::parseText(std::string_view text)
{
	if (text == "[ENABLE_CHART_DYNAMICS]")
		m_tracker.enableDynamics = true;
	else
		m_track.get_or_emplace_Events_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
}

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseText(std::string_view text)
{
	if (text == "[ENABLE_CHART_DYNAMICS]")
		m_tracker.enableDynamics = true;
	else
		m_track.get_or_emplace_Events_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
}
