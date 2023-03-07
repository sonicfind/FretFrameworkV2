#pragma once
#include "InstrumentalTrack.h"
#include "Notes/DrumNote_Legacy.h"

template <>
class DifficultyTrack<DrumNote_Legacy>
{
	friend class InstrumentalTrack<DrumNote_Legacy>;
	friend class DifficultyTrack<DrumNote<4, true>>;
	friend class DifficultyTrack<DrumNote<5, false>>;

public:
	[[nodiscard]] bool isOccupied() const;

private:
	[[nodiscard]] DrumType_Enum load_V1(TxtFileReader& reader);

	DrumNote_Legacy* construct_note_midi(uint32_t position)
	{
		if (m_notes.capacity() == 0)
			m_notes.reserve(5000);

		return m_notes.try_construct_back(position);
	}

	void construct_phrase_midi(uint32_t position)
	{
		m_specialPhrases.try_construct_back(position);
	}

	void addNote_midi(uint32_t position, size_t note, uint32_t sustain = 0)
	{
		if (sustain < 20)
			sustain = 0;

		auto iter = m_notes.end() - 1;
		while (iter->key > position)
			--iter;

		(*iter)->set(note, sustain);
	}

	void addNote(uint32_t position, size_t note, uint32_t sustain = 0)
	{
		m_notes[position].set(note, sustain);
	}

	void addSpecialPhrase_midi(uint32_t position, SpecialPhrase phrase)
	{
		auto iter = m_specialPhrases.end() - 1;
		while (iter->key > position)
			--iter;

		(*iter)->push_back(phrase);
	}

	DrumNote_Legacy& backNote_midiOnly()
	{
		return m_notes.back();
	}

	void modifyBackNote_midiOnly(uint32_t position, char modifier, size_t lane)
	{
		m_notes.back().modify(modifier, lane);
	}

	void shrink()
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}

private:
	SimpleFlatMap<DrumNote_Legacy> m_notes;
	SimpleFlatMap<std::vector<SpecialPhrase>> m_specialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_events;
};

template<>
class InstrumentalTrack<DrumNote_Legacy>
{
	friend class InstrumentalTrack<DrumNote<4, true>>;
	friend class InstrumentalTrack<DrumNote<5, false>>;
	friend class InstrumentTrackMidiParser<DrumNote_Legacy>;

public:
	void load_V1(size_t diff, TxtFileReader& reader);
	[[nodiscard]] bool isOccupied() const;
	[[nodiscard]] DrumType_Enum getDrumType() const noexcept;

private:
	void setDrumType(DrumType_Enum type) { m_drumType = type; }

private:
	DrumNote_Legacy* construct_note_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].construct_note_midi(position);
	}

	void addNote_midi(size_t diff, uint32_t position, size_t note, uint32_t sustain = 0)
	{
		m_difficulties[diff].addNote_midi(position, note, sustain);
	}

	DrumNote_Legacy& backNote_midiOnly(size_t diff)
	{
		return m_difficulties[diff].backNote_midiOnly();
	}

	void construct_phrase_midi(uint32_t position)
	{
		m_trackSpecialPhrases.try_construct_back(position);
	}

	void addEvent_midi(uint32_t position, std::string_view str)
	{
		m_trackEvents.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
	}

	void addSpecialPhrase_midi(uint32_t position, SpecialPhrase phrase)
	{
		auto iter = m_trackSpecialPhrases.end() - 1;
		while (iter->key > position)
			--iter;

		(*iter)->push_back(phrase);
	}

	void modifyBackNote_midiOnly(size_t diff, uint32_t position, char modifier, size_t lane = 0)
	{
		m_difficulties[diff].modifyBackNote_midiOnly(position, modifier, lane);
	}

	void shrink_midi()
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

	void addSharedPhrase(uint32_t position, SpecialPhrase phrase)
	{
		m_trackSpecialPhrases[position].push_back(phrase);
	}

	void addNote(size_t diffIndex, uint32_t position, size_t note, uint32_t sustain = 0)
	{
		m_difficulties[diffIndex].addNote(position, note, sustain);
	}

private:
	DifficultyTrack<DrumNote_Legacy> m_difficulties[5];
	SimpleFlatMap<std::vector<SpecialPhrase>> m_trackSpecialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_trackEvents;

	DrumType_Enum m_drumType = DrumType_Enum::LEGACY;
};

template <>
template <>
DifficultyTrack<DrumNote<4, true>>& DifficultyTrack<DrumNote<4, true>>::operator=(DifficultyTrack<DrumNote_Legacy>&& diff);

template <>
template <>
DifficultyTrack<DrumNote<5, false>>& DifficultyTrack<DrumNote<5, false>>::operator=(DifficultyTrack<DrumNote_Legacy>&& diff);
