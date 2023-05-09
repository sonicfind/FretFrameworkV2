#pragma once
#include "VocalTrack.h"
#include "Serialization/MidiFileReader.h"
#include "Song/Tracks/MidiNodeGetter.h"

template <size_t numTracks>
class Midi_Loader_Vocal
{
public:
	Midi_Loader_Vocal(VocalTrack<numTracks>& track, unsigned char multiplier) : m_track(track), m_multiplierNote(multiplier) {}

	template <size_t INDEX = 0>
	bool load(MidiFileReader& reader)
	{
		static_assert(INDEX < numTracks);
		if (m_track.hasNotes(INDEX))
			return false;

		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<INDEX, true>(note.value, midiEvent->position);
				else
					parseNote<INDEX, false>(note.value, midiEvent->position);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				parseNote<INDEX, false>(reader.extractMidiNote().value, midiEvent->position);
			else if (midiEvent->type == MidiEventType::SysEx || midiEvent->type == MidiEventType::SysEx_End)
				parseSysEx(reader.extractTextOrSysEx());
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText<INDEX>(m_lyric, reader.extractTextOrSysEx(), midiEvent->position);
		}

		if (m_lyric.first != UINT32_MAX)
			emplaceVocal<INDEX>(m_lyric.first).setLyric(m_lyric.second);
		return true;
	}

private:
	template <size_t INDEX, bool NoteOn>
	void parseNote(unsigned char midiValue, uint32_t position)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
		if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
			parseVocal<INDEX, NoteOn>(midiValue, position);
		else if constexpr (INDEX == 0)
		{
			if (midiValue == 96)
				addPercussion<true, NoteOn>(m_perc, position);
			else if (midiValue == 97)
				addPercussion<false, NoteOn>(m_perc, position);
			else if (midiValue == 105 || midiValue == 106)
				addSpecialPhrase<NoteOn>(m_lyricLine, position);
			else if (midiValue == m_multiplierNote)
				addSpecialPhrase<NoteOn>(m_starPower, position);
			else if (midiValue == 0)
				addSpecialPhrase<NoteOn>(m_rangeShift, position);
			else if (midiValue == 1)
				addSpecialPhrase<NoteOn>(m_lyricShift, position);
		}
		else if constexpr (INDEX == 1)
		{
			if (midiValue == 105 || midiValue == 106)
				addSpecialPhrase<NoteOn>(m_harmonyLine, position);
		}
	}

	template <size_t INDEX, bool NoteOn>
	void parseVocal(unsigned char pitch, uint32_t position)
	{
		if (m_vocalPos != UINT32_MAX && m_lyric.first == m_vocalPos)
		{
			uint32_t sustain = position - m_vocalPos;
			if constexpr (NoteOn)
			{
				if (sustain > 240)
					sustain -= 120;
				else
					sustain /= 2;
			}

			Vocal& vocal = emplaceVocal<INDEX>(m_vocalPos);
			vocal.setLyric(m_lyric.second);
			vocal.set(pitch, sustain);
			m_lyric.first = UINT32_MAX;
		}

		if constexpr (NoteOn)
			m_vocalPos = position;
		else
			m_vocalPos = UINT32_MAX;
	}

	template <size_t INDEX>
	void parseText(std::pair<uint32_t, std::string_view>& lyric, std::string_view text, uint32_t position)
	{
		if (text[0] != '[')
		{
			if (lyric.first != UINT32_MAX)
				emplaceVocal<INDEX>(lyric.first).setLyric(lyric.second);
			lyric = { position, text };
		}
		else if constexpr (INDEX == 0)
			m_track.m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(text));
	}

	template <size_t INDEX>
	Vocal& emplaceVocal(uint32_t position)
	{
		if (m_track[INDEX].capacity() == 0)
			m_track[INDEX].reserve(500);

		return m_track[INDEX].emplace_back(position);
	}

	template <bool NoteOn>
	void addSpecialPhrase(std::pair<SpecialPhraseType, uint32_t>& combo, uint32_t position)
	{
		if constexpr (NoteOn)
		{
			m_track.m_specialPhrases.get_or_emplace_back(position);
			combo.second = position;
		}
		else if (combo.second != UINT32_MAX)
		{
			GetNode(m_track.m_specialPhrases, combo.second)->push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	template <bool PLAYABLE, bool NoteOn>
	void addPercussion(uint32_t& percPos, uint32_t position)
	{
		if constexpr (NoteOn)
			percPos = position;
		else if (percPos != UINT32_MAX)
		{
			m_track.m_percussion.get_or_emplace_back(percPos).setPlayable(PLAYABLE);
			percPos = UINT32_MAX;
		}
	}

	void parseSysEx(std::string_view sysex)
	{
	}

private:
	const unsigned char m_multiplierNote;

	uint32_t m_perc = UINT32_MAX;
	uint32_t m_vocalPos = UINT32_MAX;
	std::pair<uint32_t, std::string_view> m_lyric{ UINT32_MAX, "" };

	std::pair<SpecialPhraseType, uint32_t> m_starPower = { SpecialPhraseType::StarPower,   UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_lyricLine = { SpecialPhraseType::LyricLine,   UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_harmonyLine = { SpecialPhraseType::HarmonyLine, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_rangeShift = { SpecialPhraseType::RangeShift,  UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> m_lyricShift = { SpecialPhraseType::LyricShift,  UINT32_MAX };

	VocalTrack<numTracks>& m_track;
	
};
