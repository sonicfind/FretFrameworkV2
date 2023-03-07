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
			{
				MidiNote note = m_reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<true, INDEX>(note);
				else
					parseNote<false, INDEX>(note);
			}
			else if (eventType == MidiEventType::Note_Off)
				parseNote<false, INDEX>(m_reader.extractMidiNote());
			else if (eventType == MidiEventType::SysEx || eventType == MidiEventType::SysEx_End)
				parseSysEx(m_reader.extractTextOrSysEx());
			else if (m_reader.getEventType() <= MidiEventType::Text_EnumLimit)
				parseText<INDEX>(m_reader.extractTextOrSysEx());
		}

		if (m_lyric.first != UINT32_MAX)
			m_track.add_lyric_midi<INDEX>(m_lyric.first, m_lyric.second);

		m_track.shrink_midi<INDEX>();
	}

private:
	template <bool ON, size_t INDEX>
	void parseNote(MidiNote note)
	{
		const uint32_t position = m_reader.getPosition();
		if (s_PITCHRANGE.first <= note.value && note.value < s_PITCHRANGE.second)
			parseVocal<ON, INDEX>(note);
		else if constexpr (INDEX == 0)
		{
			if (note.value == 96 || note.value == 97)
				addPercussion<ON>(note.value);
			else if (note.value == 105 || note.value == 106)
				addSpecialPhrase<ON>(m_lyricLine);
			else if (note.value == m_reader.getStarPowerValue())
				addSpecialPhrase<ON>(m_starPower);
			else if (note.value == 0)
				addSpecialPhrase<ON>(m_rangeShift);
			else if (note.value == 1)
				addSpecialPhrase<ON>(m_lyricShift);
		}
		else if constexpr (INDEX == 1)
		{
			if (note.value == 105 || note.value == 106)
				addSpecialPhrase<ON>(m_harmonyLine);
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
			m_track.addEvent_midi(position, text);
		else
		{
			if (m_vocalPos != UINT32_MAX)
				m_track.add_lyric_midi<INDEX>(m_vocalPos, text);
			else
			{
				if (m_lyric.first != UINT32_MAX)
					m_track.add_lyric_midi<INDEX>(m_lyric.first, m_lyric.second);
				m_lyric = { position, text };
			}
		}
	}

	template <bool ON>
	void addSpecialPhrase(ValCombo& combo)
	{
		uint32_t position = m_reader.getPosition();
		if constexpr (ON)
		{
			m_track.construct_phrase_midi(position);
			combo.second = position;
		}
		else if (combo.second != UINT32_MAX)
		{
			m_track.addSpecialPhrase_midi(combo.second, { combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	template <bool ON>
	void addSpecialPhrase(int diff, ValCombo& combo)
	{
		uint32_t position = m_reader.getPosition();
		if constexpr (ON)
		{
			m_track.construct_phrase_midi(diff, position);
			combo.second = position;
		}
		else if (combo.second != UINT32_MAX)
		{
			m_track.addSpecialPhrase_midi(diff, combo.second, { combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	template <bool ON, size_t INDEX>
	void parseVocal(MidiNote note)
	{
		uint32_t position = m_reader.getPosition();
		if constexpr (ON)
		{
			// This is a security put in place to handle poor GH rips
			if (m_vocalPos != UINT32_MAX)
			{
				if (Vocal* vocal = m_track.testBackNote_midiOnly<INDEX>(m_vocalPos))
				{
					uint32_t sustain = position - m_vocalPos;
					if (sustain > 240)
						sustain -= 120;
					else
						sustain /= 2;

					vocal->set(m_pitch, sustain);
				}
			}
			else
			{
				if (m_lyric.first == position)
					m_track.add_lyric_midi<INDEX>(position, m_lyric.second);
				m_lyric.first = UINT32_MAX;
			}

			m_vocalPos = position;
			m_pitch = note.value;
		}
		else if (m_vocalPos != UINT32_MAX)
		{
			if (Vocal* vocal = m_track.testBackNote_midiOnly<INDEX>(m_vocalPos))
				vocal->set(m_pitch, position - m_vocalPos);
			m_vocalPos = UINT32_MAX;
		}
	}

	template <bool ON>
	void toggleExtraValues(MidiNote note)
	{
	}

	template <bool ON>
	void addPercussion(int noteValue)
	{
		uint32_t position = m_reader.getPosition();
		if constexpr (ON)
			m_perc = position;
		else if (m_perc != UINT32_MAX)
		{
			m_track.add_percusssion_midi(m_perc).setPlayable(noteValue == 97);
			m_perc = UINT32_MAX;
		}
	}

private:
	static constexpr std::pair<unsigned char, unsigned char> s_PITCHRANGE = { 36, 85 };

	VocalTrack<numTracks>& m_track;
	MidiFileReader& m_reader;

	ValCombo m_starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
	ValCombo m_lyricLine = { SpecialPhraseType::LyricLine, UINT32_MAX };
	ValCombo m_harmonyLine = { SpecialPhraseType::HarmonyLine, UINT32_MAX };
	ValCombo m_rangeShift = { SpecialPhraseType::RangeShift, UINT32_MAX };
	ValCombo m_lyricShift = { SpecialPhraseType::LyricShift, UINT32_MAX };

	uint32_t m_perc = UINT32_MAX;

	uint32_t m_vocalPos = UINT32_MAX;
	unsigned char m_pitch = 0;

	std::pair<uint32_t, std::string_view> m_lyric { UINT32_MAX, ""};
};
