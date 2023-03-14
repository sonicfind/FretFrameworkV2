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

	void addNote(uint32_t position, size_t note, uint32_t sustain = 0)
	{
		m_notes[position].set(note, sustain);
	}

	DrumNote_Legacy* construct_note_midi(uint32_t position)
	{
		if (m_notes.capacity() == 0)
			m_notes.reserve(5000);

		return m_notes.try_emplace_back(position);
	}

	DrumNote_Legacy& getNote_midi(uint32_t position)
	{
		return m_notes.getNodeFromBack(position);
	}

	DrumNote_Legacy& backNote_midiOnly()
	{
		return m_notes.back();
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

	DrumNote_Legacy& getNote_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].getNote_midi(position);
	}

	DrumNote_Legacy& backNote_midiOnly(size_t diff)
	{
		return m_difficulties[diff].backNote_midiOnly();
	}

	std::vector<std::u32string>& get_or_emplace_Events_midi(uint32_t position)
	{
		return m_events.get_or_emplace_back(position);
	}

	std::vector<SpecialPhrase>& get_or_emplace_SpecialPhrase_midi(uint32_t position)
	{
		return m_specialPhrases.get_or_emplaceNodeFromBack(position);
	}

	void addNote(size_t diffIndex, uint32_t position, int note, uint32_t sustain = 0)
	{
		m_difficulties[diffIndex].addNote(position, note, sustain);
	}

	void addSharedPhrase(uint32_t position, SpecialPhrase phrase)
	{
		m_specialPhrases[position].push_back(phrase);
	}

	void shrink_midi()
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

private:
	DifficultyTrack<DrumNote_Legacy> m_difficulties[5];
	SimpleFlatMap<std::vector<SpecialPhrase>> m_specialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_events;

	DrumType_Enum m_drumType = DrumType_Enum::LEGACY;
};

template <>
template <>
DifficultyTrack<DrumNote<4, true>>& DifficultyTrack<DrumNote<4, true>>::operator=(DifficultyTrack<DrumNote_Legacy>&& diff);

template <>
template <>
DifficultyTrack<DrumNote<5, false>>& DifficultyTrack<DrumNote<5, false>>::operator=(DifficultyTrack<DrumNote_Legacy>&& diff);
