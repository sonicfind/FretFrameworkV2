#pragma once
#include "Midi_Loader_Instrument.h"
#include "Notes/GuitarNote.h"

template <>
struct Midi_Loader_Instrument::Loader_Diff<GuitarNote<5>>
{
	bool sliderNotes = false;
	bool hopoOn = false;
	bool hopoOff = false;
	Midi_Loader::PhraseNode<SpecialPhraseType> starPower = { SpecialPhraseType::StarPower_Diff };
	Midi_Loader::PhraseNode<SpecialPhraseType> faceOff[2] = { { SpecialPhraseType::FaceOff_Player1 }, { SpecialPhraseType::FaceOff_Player2 } };
	uint64_t notes[6] = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
};

template <>
struct Midi_Loader_Instrument::Loader_Diff<GuitarNote<6>>
{
	bool sliderNotes = false;
	bool hopoOn = false;
	bool hopoOff = false;
	uint64_t notes[7] = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<GuitarNote<5>>::s_noteRange{ 59, 107 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<GuitarNote<6>>::s_noteRange{ 58, 103 };

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote<5>>::Loader_Lanes();

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote<6>>::Loader_Lanes();

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::modNote(GuitarNote<5>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<6>>::modNote(GuitarNote<6>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::processExtraLanes(size_t diff, size_t lane)
{
	// HopoON marker
	if (lane == 6)
	{
		m_difficulties[diff].hopoOn = NoteOn;
		if constexpr (NoteOn)
			if (auto note = m_track[diff].m_notes.try_back(m_position))
				note->setForcing(ForceStatus::HOPO_ON);
	}
	// HopoOff marker
	else if (lane == 7)
	{
		m_difficulties[diff].hopoOff = NoteOn;
		if constexpr (NoteOn)
			if (auto note = m_track[diff].m_notes.try_back(m_position))
				note->setForcing(ForceStatus::HOPO_OFF);
	}
	else if (lane == 8)
	{
		if (diff == 3)
		{
			Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_solo, m_position, 100);
			return;
		}

		for (size_t i = 0; i < 4; ++i)
			m_lanes.values[12 * i + 8] = 12;

		for (auto iter = m_track.m_specialPhrases.begin(); iter < m_track.m_specialPhrases.end();)
		{
			for (auto phraseIter = (*iter)->begin(); phraseIter < (*iter)->end();)
			{
				if (phraseIter->type == SpecialPhraseType::Solo)
				{
					m_track[3].m_specialPhrases[iter->key].push_back({ SpecialPhraseType::StarPower_Diff, phraseIter->getLength() });
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

		Midi_Loader::AddPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].starPower, m_position, 100);
	}
	else if (lane == 9)
		m_difficulties[diff].sliderNotes = NoteOn;
	else if (lane == 10)
		Midi_Loader::AddPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].faceOff[0], m_position, 100);
	else if (lane == 11)
		Midi_Loader::AddPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].faceOff[1], m_position, 100);
	else if (lane == 12)
		Midi_Loader::AddPhrase<NoteOn>(m_track[diff].m_specialPhrases, m_difficulties[diff].starPower, m_position, 100);
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote<6>>::processExtraLanes(size_t diff, size_t lane)
{
	// HopoON marker
	if (lane == 7)
	{
		m_difficulties[diff].hopoOn = NoteOn;
		if constexpr (NoteOn)
			if (auto note = m_track[diff].m_notes.try_back(m_position))
				note->setForcing(ForceStatus::HOPO_ON);
	}
	// HopoOff marker
	else if (lane == 8)
	{
		m_difficulties[diff].hopoOff = NoteOn;
		if constexpr (NoteOn)
			if (auto note = m_track[diff].m_notes.try_back(m_position))
				note->setForcing(ForceStatus::HOPO_OFF);
	}
	else if (lane == 10)
		m_difficulties[diff].sliderNotes = NoteOn;
}

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::parseSysEx(std::string_view str);

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<6>>::parseSysEx(std::string_view str);

template <>
void Midi_Loader_Instrument::Loader<GuitarNote<5>>::parseText(std::string_view str);
