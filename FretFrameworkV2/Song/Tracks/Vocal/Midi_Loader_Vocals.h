#pragma once
#include "VocalTrack.h"
#include "Serialization/MidiFileReader.h"
#include "Song/Tracks/Midi_Loader.h"

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
			m_position = midiEvent->position;
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<INDEX, true>(note.value);
				else
					parseNote<INDEX, false>(note.value);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				parseNote<INDEX, false>(reader.extractMidiNote().value);
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
	void parseNote(unsigned char midiValue)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
		if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
			parseVocal<INDEX, NoteOn>(midiValue);
		else if constexpr (INDEX == 0)
		{
			if (midiValue == 96)
				addPercussion<true, NoteOn>(m_perc);
			else if (midiValue == 97)
				addPercussion<false, NoteOn>(m_perc);
			else if (midiValue == 105 || midiValue == 106)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_lyricLine, m_position, 100);
			else if (midiValue == m_multiplierNote)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_starPower, m_position, 100);
			else if (midiValue == 0)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_rangeShift, m_position, 100);
			else if (midiValue == 1)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_lyricShift, m_position, 100);
		}
		else if constexpr (INDEX == 1)
		{
			if (midiValue == 105 || midiValue == 106)
				Midi_Loader::AddPhrase<NoteOn>(m_track.m_specialPhrases, m_harmonyLine, m_position, 100);
		}
	}

	template <size_t INDEX, bool NoteOn>
	void parseVocal(unsigned char pitch)
	{
		if (m_vocalPos != UINT64_MAX && m_lyric.first == m_vocalPos)
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
			m_vocalPos = m_position;
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
			m_lyric = { m_position, text };
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
	const unsigned char m_multiplierNote;

	uint64_t m_position = 0;
	uint64_t m_perc = UINT64_MAX;
	uint64_t m_vocalPos = UINT64_MAX;
	std::pair<uint64_t, std::string_view> m_lyric{ UINT64_MAX, "" };

	Midi_Loader::PhraseNode<SpecialPhraseType> m_starPower =   { SpecialPhraseType::StarPower };
	Midi_Loader::PhraseNode<SpecialPhraseType> m_lyricLine =   { SpecialPhraseType::LyricLine };
	Midi_Loader::PhraseNode<SpecialPhraseType> m_harmonyLine = { SpecialPhraseType::HarmonyLine };
	Midi_Loader::PhraseNode<SpecialPhraseType> m_rangeShift =  { SpecialPhraseType::RangeShift };
	Midi_Loader::PhraseNode<SpecialPhraseType> m_lyricShift =  { SpecialPhraseType::LyricShift };

	VocalTrack<numTracks>& m_track;
	
};
