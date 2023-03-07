#pragma once
#include "InstrumentalTrack_Midi.h"
#include "Notes/GuitarNote.h"

template <>
struct Tracker<GuitarNote<5>>
{
	using ValCombo = std::pair<SpecialPhraseType, uint32_t>;
	struct
	{
		bool sliderNotes = false;
		bool hopoOn = false;
		bool hopoOff = false;
		ValCombo starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
		ValCombo faceOff[2] = { { SpecialPhraseType::FaceOff_Player1, UINT32_MAX } , { SpecialPhraseType::FaceOff_Player2, UINT32_MAX } };
		uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	} difficulties[5];
};

template <>
struct Tracker<GuitarNote<6>>
{
	struct
	{
		bool sliderNotes = false;
		bool hopoOn = false;
		bool hopoOff = false;
		uint32_t notes[7] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	} difficulties[5];
};

template <>
template <bool ON>
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
		if constexpr (ON)
		{
			m_tracker.difficulties[diff].notes[lane] = position;

			if (GuitarNote<5>* guitar = m_track.construct_note_midi(diff, position))
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
			const uint32_t colorPosition = m_tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.addNote_midi(diff, colorPosition, lane, position - colorPosition);
				m_tracker.difficulties[diff].notes[lane] = UINT32_MAX;
			}
		}
	}
	// HopoON marker
	else if (lane == 6)
	{
		m_tracker.difficulties[diff].hopoOn = ON;
		if constexpr (ON)
		{
			if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(diff, position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 7)
	{
		m_tracker.difficulties[diff].hopoOff = ON;
		if constexpr (ON)
		{
			if (GuitarNote<5>*note = m_track.testBackNote_midiOnly(diff, position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
	else if (lane == 8)
	{
		if (diff == 3)
		{
			addSpecialPhrase<ON>(m_solo);
			return;
		}

		m_track.convertSoloesToStarPower_midi();
		addSpecialPhrase<ON>(diff, m_tracker.difficulties[diff].starPower);

		for (size_t i = 0; i < 4; ++i)
			m_laneValues[12 * i + 8] = 12;
	}
	else if (lane == 9)
		m_tracker.difficulties[diff].sliderNotes = ON;
	else if (lane == 10)
		addSpecialPhrase<ON>(diff, m_tracker.difficulties[diff].faceOff[0]);
	else if (lane == 11)
		addSpecialPhrase<ON>(diff, m_tracker.difficulties[diff].faceOff[1]);
	else if (lane == 12)
		addSpecialPhrase<ON>(diff, m_tracker.difficulties[diff].starPower);
}

template <>
template <bool ON>
void InstrumentTrackMidiParser<GuitarNote<6>>::parseLaneColor(MidiNote note)
{
	const int noteValue = note.value - m_noteRange.first;
	const int lane = m_laneValues[noteValue];
	const int diff = s_diffValues[noteValue];
	const uint32_t position = m_reader.getPosition();

	if (lane < 7)
	{
		if constexpr (ON)
		{
			m_tracker.difficulties[diff].notes[lane] = position;

			if (GuitarNote<6>*guitar = m_track.construct_note_midi(diff, position))
			{
				if (m_tracker.difficulties[diff].sliderNotes)
					guitar->modify('T', false);

				if (m_tracker.difficulties[diff].hopoOn)
					guitar->modify('<');
				else if (m_tracker.difficulties[diff].hopoOff)
					guitar->modify('>');
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
	// HopoON marker
	else if (lane == 6)
	{
		m_tracker.difficulties[diff].hopoOn = ON;
		if constexpr (ON)
		{
			if (GuitarNote<6>*note = m_track.testBackNote_midiOnly(diff, position))
				note->modify('>');
		}
	}
	// HopoOff marker
	else if (lane == 7)
	{
		m_tracker.difficulties[diff].hopoOff = ON;
		if constexpr (ON)
		{
			if (GuitarNote<6>*note = m_track.testBackNote_midiOnly(diff, position))
				note->modify('<');
		}
	}
}

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::init();

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseSysEx(std::string_view sysex);

template <>
void InstrumentTrackMidiParser<GuitarNote<5>>::parseText(std::string_view text);

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::init();

template <>
void InstrumentTrackMidiParser<GuitarNote<6>>::parseSysEx(std::string_view sysex);
