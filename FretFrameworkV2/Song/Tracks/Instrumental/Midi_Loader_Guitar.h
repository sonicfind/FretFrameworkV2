#pragma once
#include "Midi_Loader_Instrument.h"
#include "Notes/GuitarNote.h"

template <>
struct Midi_Loader_Diff<GuitarNote<5>>
{
	bool sliderNotes = false;
	bool hopoOn = false;
	bool hopoOff = false;
	std::pair<SpecialPhraseType, uint32_t> starPower = { SpecialPhraseType::StarPower_Diff, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> faceOff[2] = { { SpecialPhraseType::FaceOff_Player1, UINT32_MAX } , { SpecialPhraseType::FaceOff_Player2, UINT32_MAX } };
	uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
struct Midi_Loader_Diff<GuitarNote<6>>
{
	bool sliderNotes = false;
	bool hopoOn = false;
	bool hopoOff = false;
	uint32_t notes[7] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader<GuitarNote<5>>::s_noteRange{ 59, 107 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader<GuitarNote<6>>::s_noteRange{ 58, 103 };

template <>
constexpr size_t Midi_Loader<GuitarNote<5>>::s_defaultLanes[48] =
{
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

template <>
constexpr size_t Midi_Loader<GuitarNote<6>>::s_defaultLanes[48] =
{
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
};

template <>
void Midi_Loader<GuitarNote<5>>::modNote(GuitarNote<5>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
void Midi_Loader<GuitarNote<6>>::modNote(GuitarNote<6>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
template <bool NoteOn>
void Midi_Loader<GuitarNote<5>>::processExtraLanes(size_t diff, size_t lane, uint32_t position)
{
	// HopoON marker
	if (lane == 6)
	{
		m_difficulties[diff].hopoOn = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_track[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 7)
	{
		m_difficulties[diff].hopoOff = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_track[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
	else if (lane == 8)
	{
		if (diff == 3)
		{
			addSpecialPhrase<NoteOn>(m_track.m_specialPhrases, m_solo, position);
			return;
		}

		for (size_t i = 0; i < 4; ++i)
			m_laneValues[12 * i + 8] = 12;

		for (auto iter = m_track.m_specialPhrases.begin(); iter < m_track.m_specialPhrases.end();)
		{
			for (auto phraseIter = (*iter)->begin(); phraseIter < (*iter)->end();)
			{
				if (phraseIter->getMidiNote() == 103)
				{
					m_track[3].m_specialPhrases[iter->key].push_back({ SpecialPhraseType::StarPower_Diff, phraseIter->getDuration() });
					phraseIter = (*iter)->erase(phraseIter);
				}
				else
					++phraseIter;
			}

			if ((*iter)->empty())
				iter = m_track.m_specialPhrases.erase(iter);
			else
				++iter;
		}

		addSpecialPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].starPower, position);
	}
	else if (lane == 9)
		m_difficulties[diff].sliderNotes = NoteOn;
	else if (lane == 10)
		addSpecialPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].faceOff[0], position);
	else if (lane == 11)
		addSpecialPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].faceOff[1], position);
	else if (lane == 12)
		addSpecialPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].starPower, position);
}

template <>
template <bool NoteOn>
void Midi_Loader<GuitarNote<6>>::processExtraLanes(size_t diff, size_t lane, uint32_t position)
{
	// HopoON marker
	if (lane == 7)
	{
		m_difficulties[diff].hopoOn = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_track[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 8)
	{
		m_difficulties[diff].hopoOff = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_track[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
	else if (lane == 10)
		m_difficulties[diff].sliderNotes = NoteOn;
}

template <>
void Midi_Loader<GuitarNote<5>>::parseSysEx(std::string_view str, uint32_t position);

template <>
void Midi_Loader<GuitarNote<6>>::parseSysEx(std::string_view str, uint32_t position);

template <>
void Midi_Loader<GuitarNote<5>>::parseText(std::string_view str, uint32_t position);
