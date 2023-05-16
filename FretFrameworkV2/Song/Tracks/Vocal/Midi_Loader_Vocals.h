#pragma once
#include "VocalTrack.h"
#include "Serialization/MidiFileReader.h"
#include "Song/Tracks/Midi_Loader.h"

template <size_t numTracks>
class Midi_Loader_Vocal
{
public:
	Midi_Loader_Vocal(VocalTrack<numTracks>& track, unsigned char multiplier)
		: m_track(track), m_phrases({
			{ { multiplier }, { SpecialPhraseType::StarPower } },
			{ { 105, 106 }, { SpecialPhraseType::LyricLine } },
			{ { 0 }, { SpecialPhraseType::RangeShift } },
			{ { 1 }, { SpecialPhraseType::LyricShift } },
			{ { 0xFF }, { SpecialPhraseType::HarmonyLine } },
		}) {}

	template <size_t INDEX = 0>
	bool load(MidiFileReader& reader)
	{
		static_assert(INDEX < numTracks);
		if (m_track.hasNotes(INDEX))
			return false;

		while (auto midiEvent = reader.parseEvent())
		{
			m_position = midiEvent->position;
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<INDEX, true>(note);
				else
					parseNote<INDEX, false>(note);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				parseNote<INDEX, false>(reader.extractMidiNote());
			else if (midiEvent->type == MidiEventType::SysEx || midiEvent->type == MidiEventType::SysEx_End)
				parseSysEx(reader.extractTextOrSysEx());
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText<INDEX>(reader.extractTextOrSysEx());
		}

		if (m_lyric.first != UINT64_MAX)
			emplaceVocal<INDEX>(m_lyric.first);
		return true;
	}

private:
	template <size_t INDEX, bool NoteOn>
	void parseNote(MidiNote note)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 84 };
		if (PITCHRANGE.first <= note.value && note.value <= PITCHRANGE.second)
			parseVocal<INDEX, NoteOn>(note.value);
		else if constexpr (INDEX == 0)
		{
			if (note.value == 96)
				addPercussion<true, NoteOn>(m_perc);
			else if (note.value == 97)
				addPercussion<false, NoteOn>(m_perc);
			else
				m_phrases.addPhrase<NoteOn>(m_track.m_specialPhrases, m_position, note);
		}
		else if constexpr (INDEX == 1)
		{
			if (note.value == 105 || note.value == 106)
				m_phrases.addPhrase<NoteOn>(m_track.m_specialPhrases, m_position, SpecialPhraseType::HarmonyLine, 100);
		}
	}

	template <size_t INDEX, bool NoteOn>
	void parseVocal(unsigned char pitch)
	{
		if (m_vocalPos != UINT64_MAX)
		{
			uint64_t sustain = m_position - m_vocalPos;
			if constexpr (NoteOn)
			{
				if (sustain > 240)
					sustain -= 120;
				else
					sustain /= 2;
			}

			Vocal& vocal = emplaceVocal<INDEX>(m_vocalPos);
			vocal.pitch.set(pitch, sustain);
			m_lyric.first = UINT64_MAX;
		}

		if constexpr (NoteOn)
		{
			m_vocalPos = m_position;
			if (m_lyric.first != UINT64_MAX)
				m_lyric.first = m_position;
		}
		else
			m_vocalPos = UINT64_MAX;
	}

	template <size_t INDEX>
	void parseText(std::string_view text)
	{
		if (text[0] != '[')
		{
			if (m_lyric.first != UINT64_MAX)
				emplaceVocal<INDEX>(m_lyric.first);
			m_lyric = { m_vocalPos != UINT64_MAX ? m_vocalPos : m_position, text };
		}
		else if constexpr (INDEX == 0)
			m_track.m_events.get_or_emplace_back(m_position).push_back(UnicodeString::strToU32(text));
	}

	template <size_t INDEX>
	Vocal& emplaceVocal(uint64_t position)
	{
		if (m_track[INDEX].capacity() == 0)
			m_track[INDEX].reserve(500);

		return m_track[INDEX].emplace_back(position, { UnicodeString::strToU32(m_lyric.second) });
	}

	template <bool PLAYABLE, bool NoteOn>
	void addPercussion(uint64_t& percPos)
	{
		if constexpr (NoteOn)
			percPos = m_position;
		else if (percPos != UINT64_MAX)
		{
			m_track.m_percussion.get_or_emplace_back(percPos).setPlayable(PLAYABLE);
			percPos = UINT64_MAX;
		}
	}

	void parseSysEx(std::string_view sysex)
	{
	}

private:
	uint64_t m_position = 0;
	uint64_t m_perc = UINT64_MAX;
	uint64_t m_vocalPos = UINT64_MAX;
	std::pair<uint64_t, std::string_view> m_lyric{ UINT64_MAX, "" };

	Midi_Loader::Loader_Phrases<SpecialPhraseType> m_phrases;

	VocalTrack<numTracks>& m_track;
	
};
