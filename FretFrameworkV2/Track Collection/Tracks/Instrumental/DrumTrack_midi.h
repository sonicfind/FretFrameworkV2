#pragma once
#include "InstrumentalTrack_Midi.h"
#include "InstrumentalTrack_DrumsLegacy.h"

template <size_t numPads, bool PRO_DRUMS>
struct Tracker<DrumNote<numPads, PRO_DRUMS>>
{
	struct Diff
	{
		bool flam = false;
		uint32_t notes[numPads + 1];
		Diff() { for (uint32_t& n : notes) n = UINT32_MAX; }
	} difficulties[5];

	bool enableDynamics = false;
	bool toms[3] = { !PRO_DRUMS, !PRO_DRUMS, !PRO_DRUMS };
};

template <>
struct Tracker<DrumNote_Legacy> : Tracker<DrumNote<5, true>> {};

template <>
template <bool ON>
void InstrumentTrackMidiParser<DrumNote<4, true>>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 5)
	{
		if constexpr (ON)
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
			const uint32_t colorPosition = m_tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.addNote_midi(diff, colorPosition, lane, position - colorPosition);
				m_tracker.difficulties[diff].notes[lane] = UINT32_MAX;
			}
		}
	}
	else if (note.value == 95)
	{
		if constexpr (ON)
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
				m_track.addNote_midi(3, colorPosition, 0, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
}

template <>
template <bool ON>
void InstrumentTrackMidiParser<DrumNote<5, false>>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 6)
	{
		if constexpr (ON)
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
			const uint32_t colorPosition = m_tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.addNote_midi(diff, colorPosition, lane, position - colorPosition);
				m_tracker.difficulties[diff].notes[lane] = UINT32_MAX;
			}
		}
	}
	else if (note.value == 95)
	{
		if constexpr (ON)
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
				m_track.addNote_midi(3, colorPosition, 0, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
}

template <>
template <bool ON>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 6)
	{
		if constexpr (ON)
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
			const uint32_t colorPosition = m_tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.addNote_midi(diff, colorPosition, lane, position - colorPosition);
				m_tracker.difficulties[diff].notes[lane] = UINT32_MAX;
			}
		}
	}
	else if (note.value == 95)
	{
		if constexpr (ON)
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
				m_track.addNote_midi(3, colorPosition, 0, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
}

template <>
template <bool ON>
void InstrumentTrackMidiParser<DrumNote<4, true>>::toggleExtraValues(MidiNote note)
{
	if (110 <= note.value && note.value <= 112)
		m_tracker.toms[note.value - 110] = ON;
}

template <>
template <bool ON>
void InstrumentTrackMidiParser<DrumNote_Legacy>::toggleExtraValues(MidiNote note)
{
	if (m_track.getDrumType() != DrumType_Enum::FIVELANE && 110 <= note.value && note.value <= 112)
	{
		m_tracker.toms[note.value - 110] = ON;
		m_track.setDrumType(DrumType_Enum::FOURLANE_PRO);
	}
}

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::parseText(std::string_view text);

template <>
void InstrumentTrackMidiParser<DrumNote<5, false>>::init();

template <>
void InstrumentTrackMidiParser<DrumNote_Legacy>::init();
