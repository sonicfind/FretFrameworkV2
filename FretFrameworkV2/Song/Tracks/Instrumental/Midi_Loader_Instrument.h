#pragma once
#include "InstrumentalTrack.h"
#include "Serialization/MidiFileReader.h"
#include "Song/Tracks/MidiNodeGetter.h"

template <class T>
struct Midi_Loader_Diff
{
	uint32_t notes[T::GetLaneCount()];
	constexpr Midi_Loader_Diff() { for (uint32_t& note : notes) note = UINT32_MAX; }
};

template <class T>
struct Midi_Loader_Extensions {};

template <class T>
struct Midi_Loader
{
public:
	Midi_Loader(InstrumentalTrack<T>& track, unsigned char multiplier) : m_track(track), m_multiplierNote(multiplier) {}

	bool scan(MidiFileReader& reader)
	{
		if (m_track.isOccupied())
			return false;

		memcpy(m_laneValues, s_defaultLanes, sizeof(s_defaultLanes));
		while (const auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<true>(note, midiEvent->position);
				else
					parseNote<false>(note, midiEvent->position);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				parseNote<false>(reader.extractMidiNote(), midiEvent->position);
			else if (midiEvent->type == MidiEventType::SysEx || midiEvent->type == MidiEventType::SysEx_End)
				parseSysEx(reader.extractTextOrSysEx(), midiEvent->position);
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText(reader.extractTextOrSysEx(), midiEvent->position);
		}

		m_track.shrink();
		return true;
	}

public:
	Midi_Loader_Extensions<T> m_ext;

private:
	template <bool NoteOn>
	void parseNote(MidiNote note, uint32_t position)
	{
		if (processSpecialNote<NoteOn>(note, position))
			return;

		if (s_noteRange.first <= note.value && note.value <= s_noteRange.second)
			parseLaneColor<NoteOn>(note, position);
		else if (120 <= note.value && note.value <= 124)
			parseBRE<NoteOn>(note.value, position);
		else if (note.value == m_multiplierNote)
			addSpecialPhrase<NoteOn>(m_track.m_specialPhrases, m_starPower, position);
		else if (note.value == 103)
			addSpecialPhrase<NoteOn>(m_track.m_specialPhrases, m_solo, position);
		else if (note.value == 126)
			addSpecialPhrase<NoteOn>(m_track.m_specialPhrases, m_tremolo, position);
		else if (note.value == 127)
			addSpecialPhrase<NoteOn>(m_track.m_specialPhrases, m_trill, position);
		else
			toggleExtraValues<NoteOn>(note, position);
	}

	template <bool NoteOn>
	bool processSpecialNote(MidiNote note, uint32_t position) { return false; }

	template <bool NoteOn>
	void parseLaneColor(MidiNote note, uint32_t position)
	{
		const int noteValue = note.value - s_noteRange.first;
		const size_t lane = m_laneValues[noteValue];
		const size_t diff = s_diffValues[noteValue];

		if (lane < T::GetLaneCount())
		{
			if constexpr (NoteOn)
			{
				m_difficulties[diff].notes[lane] = position;
				if (m_track[diff].m_notes.capacity() == 0)
					m_track[diff].m_notes.reserve(5000);

				modNote(constructNote(m_track[diff], position), diff, lane, note.velocity);
			}
			else
				addColor(m_track[diff].m_notes, diff, lane, position);
		}
		else
			processExtraLanes<NoteOn>(diff, lane, position);
	}

	T& constructNote(DifficultyTrack<T>& diff, uint32_t position)
	{
		if (diff.m_notes.capacity() == 0)
			diff.m_notes.reserve(5000);

		return diff.m_notes.get_or_emplace_back(position);
	}

	void modNote(T& note, size_t diff, size_t lane, unsigned char velocity) {}

	void addColor(SimpleFlatMap<T>& notes, size_t diff, size_t lane, uint32_t position)
	{
		uint32_t colorPosition = m_difficulties[diff].notes[lane];
		if (colorPosition != UINT32_MAX)
		{
			GetNode(notes, colorPosition)->set(lane, position - colorPosition);
			m_difficulties[diff].notes[lane] = UINT32_MAX;
		}
	}

	template <bool NoteOn>
	void processExtraLanes(size_t diff, size_t lane, uint32_t position) {}

	template <bool NoteOn>
	void parseBRE(uint32_t midiValue, uint32_t position)
	{
		if constexpr (NoteOn)
		{
			m_notes_BRE[midiValue - 120] = position;
			m_doBRE = m_notes_BRE[0] == m_notes_BRE[1] && m_notes_BRE[1] == m_notes_BRE[2] && m_notes_BRE[2] == m_notes_BRE[3];
		}
		else if (m_doBRE)
		{
			uint32_t colorPosition = m_notes_BRE[0];
			m_track.m_specialPhrases[colorPosition].push_back({ SpecialPhraseType::StarPowerActivation, position - colorPosition });

			for (size_t i = 0; i < 5; ++i)
				m_notes_BRE[i] = UINT32_MAX;
			m_doBRE = false;
		}
		else
		{
			const int lane = midiValue - 120;
			uint32_t colorPosition = m_notes_BRE[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track[4].m_notes[colorPosition].set(lane, position - colorPosition);
				m_notes_BRE[lane] = UINT32_MAX;
			}
		}
	}

	void parseSysEx(std::string_view str, uint32_t position) {}
	void parseText(std::string_view str, uint32_t position)
	{
		m_track.m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
	}

	template <bool NoteOn>
	void toggleExtraValues(MidiNote note, uint32_t position) {}

	template <bool NoteOn>
	void addSpecialPhrase(SimpleFlatMap<std::vector<SpecialPhrase>>& phrases, std::pair<SpecialPhraseType, uint32_t>& combo, uint32_t position)
	{
		if constexpr (NoteOn)
		{
			phrases.try_emplace_back(position);
			combo.second = position;
		}
		else if (combo.second != UINT32_MAX)
		{
			GetNode(phrases, combo.second)->push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

private:
	static constexpr std::pair<unsigned char, unsigned char> s_noteRange{ 60, 100 };
	static constexpr size_t s_diffValues[48] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
	};

	static constexpr size_t s_defaultLanes[48] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	};

	const unsigned char m_multiplierNote;

	size_t m_laneValues[48];
	uint32_t m_notes_BRE[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	bool m_doBRE = false;

	std::pair<SpecialPhraseType, uint32_t> m_solo = { SpecialPhraseType::Solo, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_tremolo = { SpecialPhraseType::Tremolo, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_trill = { SpecialPhraseType::Trill, UINT32_MAX };

	Midi_Loader_Diff<T> m_difficulties[4];
	InstrumentalTrack<T>& m_track;
};
