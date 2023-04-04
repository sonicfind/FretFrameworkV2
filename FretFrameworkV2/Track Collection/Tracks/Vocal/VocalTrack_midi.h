#pragma once
#include "VocalTrack.h"
#include "../File Processing/MidiFileReader.h"

template <size_t numTracks>
class VocalTrackMidiParser
{
	using ValCombo = std::pair<SpecialPhraseType, uint32_t>;
public:
	VocalTrackMidiParser(VocalTrack<numTracks>& track, MidiFileReader& reader)
		: m_track(track), m_reader(reader) {}

	template <size_t INDEX>
	void loadFromMidi()
	{
		static_assert(INDEX < numTracks);
		if (m_track.hasNotes<INDEX>())
			throw std::runtime_error("Track defined multiple times in file");

		while (m_reader.parseEvent())
		{
			MidiEventType eventType = m_reader.getEventType();
			if (eventType == MidiEventType::Note_On)
				parseNote<INDEX, true>(m_reader.extractMidiNote());
			else if (eventType == MidiEventType::Note_Off)
				parseNote<INDEX, false>(m_reader.extractMidiNote());
			else if (eventType == MidiEventType::SysEx || eventType == MidiEventType::SysEx_End)
				parseSysEx(m_reader.extractTextOrSysEx());
			else if (m_reader.getEventType() <= MidiEventType::Text_EnumLimit)
				parseText<INDEX>(m_reader.extractTextOrSysEx());
		}

		if (m_lyric.first != UINT32_MAX)
			m_track.getVocal_midi<INDEX>(m_lyric.first).setLyric(m_lyric.second);

		m_track.shrink_midi<INDEX>();
	}

private:
	template <size_t INDEX, bool NoteOn>
	void parseNote(MidiNote note)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };

		m_inOnState = NoteOn && note.velocity > 0;
		const uint32_t position = m_reader.getPosition();
		if (PITCHRANGE.first <= note.value && note.value < PITCHRANGE.second)
			parseVocal<INDEX>(note);
		else if constexpr (INDEX == 0)
		{
			if (note.value == 96 || note.value == 97)
				addPercussion(note.value);
			else if (note.value == 105 || note.value == 106)
				addSpecialPhrase(m_lyricLine);
			else if (note.value == m_reader.getStarPowerValue())
				addSpecialPhrase(m_starPower);
			else if (note.value == 0)
				addSpecialPhrase(m_rangeShift);
			else if (note.value == 1)
				addSpecialPhrase(m_lyricShift);
		}
		else if constexpr (INDEX == 1)
		{
			if (note.value == 105 || note.value == 106)
				addSpecialPhrase(m_harmonyLine);
		}
	}

	void parseSysEx(std::string_view sysex)
	{
	}

	template <size_t INDEX>
	void parseText(std::string_view text)
	{
		uint32_t position = m_reader.getPosition();
		if (text[0] == '[')
			m_track.get_or_emplace_Events_midi(position).push_back(UnicodeString::strToU32(text));
		else
		{
			if (m_lyric.first != UINT32_MAX)
				m_track.getVocal_midi<INDEX>(m_lyric.first).setLyric(m_lyric.second);
			m_lyric = { position, text };
		}
	}

	void addSpecialPhrase(ValCombo& combo)
	{
		uint32_t position = m_reader.getPosition();
		if (m_inOnState)
			combo.second = position;
		else if (combo.second != UINT32_MAX)
		{
			m_track.get_or_emplace_SpecialPhrase_midi(combo.second).push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	template <size_t INDEX>
	void parseVocal(MidiNote note)
	{
		uint32_t position = m_reader.getPosition();
		if (m_vocalPos != UINT32_MAX && m_lyric.first == m_vocalPos)
		{
			uint32_t sustain = position - m_vocalPos;
			if (m_inOnState)
			{
				if (sustain > 240)
					sustain -= 120;
				else
					sustain /= 2;
			}

			Vocal& vocal = m_track.getVocal_midi<INDEX>(m_vocalPos);
			vocal.setLyric(m_lyric.second);
			vocal.set(m_pitch, sustain);
			m_lyric.first = UINT32_MAX;
		}
		
		if (m_inOnState)
		{
			m_vocalPos = position;
			m_pitch = note.value;
		}
		else
			m_vocalPos = UINT32_MAX;
	}

	void addPercussion(int noteValue)
	{
		uint32_t position = m_reader.getPosition();
		if (m_inOnState)
			m_perc = position;
		else if (m_perc != UINT32_MAX)
		{
			m_track.getPercusssion_midi(m_perc).setPlayable(noteValue == 97);
			m_perc = UINT32_MAX;
		}
	}

private:
	VocalTrack<numTracks>& m_track;
	MidiFileReader& m_reader;

	ValCombo m_starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
	ValCombo m_lyricLine = { SpecialPhraseType::LyricLine, UINT32_MAX };
	ValCombo m_harmonyLine = { SpecialPhraseType::HarmonyLine, UINT32_MAX };
	ValCombo m_rangeShift = { SpecialPhraseType::RangeShift, UINT32_MAX };
	ValCombo m_lyricShift = { SpecialPhraseType::LyricShift, UINT32_MAX };
	bool m_inOnState = false;

	uint32_t m_perc = UINT32_MAX;

	uint32_t m_vocalPos = UINT32_MAX;
	unsigned char m_pitch = 0;

	std::pair<uint32_t, std::string_view> m_lyric { UINT32_MAX, ""};
};
