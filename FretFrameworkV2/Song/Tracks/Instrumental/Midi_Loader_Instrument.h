#pragma once
#include "InstrumentalTrack.h"
#include "Serialization/MidiFileReader.h"
#include "Song/Tracks/Midi_Loader.h"

namespace Midi_Loader_Instrument
{
	template <class T>
	T& ConstructNote(DifficultyTrack<T>& diff, uint64_t position)
	{
		if (diff.m_notes.capacity() == 0)
			diff.m_notes.reserve(5000);

		return diff.m_notes.get_or_emplace_back(position);
	}

	template <class T>
	struct Loader_Diff
	{
		uint64_t notes[T::GetLaneCount()];
		constexpr Loader_Diff() { for (uint64_t& note : notes) note = UINT64_MAX; }
	};

	template <class T>
	struct Loader_Ext {};

	template <class T>
	struct Loader
	{
	public:
		Loader(InstrumentalTrack<T>& track, unsigned char multiplierNote) : m_track(track), m_multiplierNote(multiplierNote)
		{
			memcpy(m_laneValues, s_defaultLanes, s_numValues * sizeof(size_t));
		}

		void setPosition(uint64_t position) { m_position = position; }

		template <bool NoteOn>
		void parseNote(MidiNote note, unsigned char channel)
		{
			if (processSpecialNote<NoteOn>(note))
				return;

			if (s_noteRange.first <= note.value && note.value <= s_noteRange.second)
				parseLaneColor<NoteOn>(note, channel);
			else if (120 <= note.value && note.value <= 124)
				parseBRE<NoteOn>(note.value);
			else if (note.value == m_multiplierNote)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_starPower, m_position);
			else if (note.value == s_soloValue)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_solo, m_position);
			else if (note.value == 126)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_tremolo, m_position);
			else if (note.value == 127)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_trill, m_position);
			else
				toggleExtraValues<NoteOn>(note);
		}

		void parseSysEx(std::string_view str) {}
		void parseText(std::string_view str)
		{
			m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(str));
		}

	private:
		template <bool NoteOn>
		bool processSpecialNote(MidiNote note) { return false; }

		template <bool NoteOn>
		void parseLaneColor(MidiNote note, unsigned char channel)
		{
			const size_t noteValue = note.value - s_noteRange.first;
			const size_t lane = m_laneValues[noteValue];
			const size_t diff = getDifficulty(noteValue);

			if (lane < T::GetLaneCount())
			{
				if constexpr (NoteOn)
				{
					m_difficulties[diff].notes[lane] = m_position;
					modNote(ConstructNote(m_track[diff], m_position), diff, lane, note.velocity);
				}
				else
					addColor(m_track[diff].m_notes, diff, lane);
			}
			else
				processExtraLanes<NoteOn>(diff, lane);
		}

		size_t getDifficulty(size_t noteValue) const noexcept { return s_diffValues[noteValue]; }

		void modNote(T& note, size_t diff, size_t lane, unsigned char velocity) {}

		void addColor(SimpleFlatMap<T>& notes, size_t diff, size_t lane)
		{
			uint64_t colorPosition = m_difficulties[diff].notes[lane];
			if (colorPosition != UINT64_MAX)
			{
				Midi_Loader::GetNode(notes, colorPosition)->set(lane, m_position - colorPosition);
				m_difficulties[diff].notes[lane] = UINT64_MAX;
			}
		}

		template <bool NoteOn>
		void processExtraLanes(size_t diff, size_t lane) {}

		template <bool NoteOn>
		void parseBRE(uint32_t midiValue)
		{
			if constexpr (NoteOn)
			{
				m_notes_BRE[midiValue - 120] = m_position;
				m_doBRE = m_notes_BRE[0] == m_notes_BRE[1] && m_notes_BRE[1] == m_notes_BRE[2] && m_notes_BRE[2] == m_notes_BRE[3];
			}
			else if (m_doBRE)
			{
				m_track.m_specialPhrases[m_notes_BRE[0]].push_back({ SpecialPhraseType::StarPowerActivation_or_BRE, m_position - m_notes_BRE[0] });

				for (uint64_t& note : m_notes_BRE)
					note = UINT64_MAX;
				m_doBRE = false;
			}
		}

		template <bool NoteOn>
		void toggleExtraValues(MidiNote note) {}

	private:
		static constexpr std::pair<unsigned char, unsigned char> s_noteRange{ 60, 100 };
		static constexpr size_t s_numValues = 48;
		static constexpr size_t s_diffValues[96] =
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
		};

		static constexpr size_t s_defaultLanes[96] =
		{
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		};

		static constexpr unsigned char s_soloValue = 103;

		const unsigned char m_multiplierNote;

		uint64_t m_position = 0;
		size_t m_laneValues[96];
		uint64_t m_notes_BRE[5] = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
		bool m_doBRE = false;

		std::pair<SpecialPhraseType, uint64_t> m_solo = { SpecialPhraseType::Solo, UINT64_MAX };
		std::pair<SpecialPhraseType, uint64_t> m_starPower = { SpecialPhraseType::StarPower, UINT64_MAX };
		std::pair<SpecialPhraseType, uint64_t> m_tremolo = { SpecialPhraseType::Tremolo, UINT64_MAX };
		std::pair<SpecialPhraseType, uint64_t> m_trill = { SpecialPhraseType::Trill, UINT64_MAX };

		Loader_Diff<T> m_difficulties[4];
		InstrumentalTrack<T>& m_track;
		Loader_Ext<T> m_ext;
	};

	template <class T>
	bool Load(InstrumentalTrack<T>& track, MidiFileReader& reader)
	{
		if (track.isOccupied())
			return false;

		Loader<T> loader(track, reader.getMultiplierNote());
		while (const auto midiEvent = reader.parseEvent())
		{
			loader.setPosition(midiEvent->position);
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					loader.parseNote<true>(note, midiEvent->channel);
				else
					loader.parseNote<false>(note, midiEvent->channel);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				loader.parseNote<false>(reader.extractMidiNote(), midiEvent->channel);
			else if (midiEvent->type == MidiEventType::SysEx || midiEvent->type == MidiEventType::SysEx_End)
				loader.parseSysEx(reader.extractTextOrSysEx());
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				loader.parseText(reader.extractTextOrSysEx());
		}

		track.shrink();
		return true;
	}
}

