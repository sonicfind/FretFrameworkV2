#pragma once
#include "ScanValues.h"
#include "Serialization/MidiFileReader.h"

template<size_t numTracks>
struct Midi_Scanner_Vocal
{
	uint32_t m_vocal = UINT32_MAX;
	uint32_t m_lyric = UINT32_MAX;

	uint32_t m_currLine = UINT32_MAX;
	std::vector<std::pair<uint32_t, uint32_t>> m_lyriclines;

	ScanValues& m_values;
	Midi_Scanner_Vocal(ScanValues& values) : m_values(values) {}

	template <size_t INDEX>
	void scan(MidiFileReader& reader)
	{
		static_assert(INDEX < numTracks);
		if (m_values.wasTrackValidated(INDEX))
			return;

		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On || midiEvent->type == MidiEventType::Note_Off)
			{
				MidiNote note = reader.extractMidiNote();
				if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
					parseNote<INDEX, true>(note.value, midiEvent->position);
				else if (parseNote<INDEX, false>(note.value, midiEvent->position))
				{
					if constexpr (numTracks == 1 || INDEX > 0)
						return;
				}
			}
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit && m_values.wasTrackValidated(INDEX))
				parseText_midi(reader.extractTextOrSysEx(), midiEvent->position);
		}
	}

	template <size_t INDEX, bool NoteOn>
	bool parseNote(unsigned char midiValue, uint32_t position)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
		if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
		{
			if (!m_values.wasTrackValidated(INDEX))
				return parseVocal<INDEX, NoteOn>(midiValue, position);
		}
		else if constexpr (INDEX == 0)
		{
			if (midiValue == 105 || midiValue == 106)
			{
				if constexpr (NoteOn)
					m_currLine = position;
				else if (m_currLine != UINT32_MAX)
				{
					if constexpr (numTracks > 1)
						m_lyriclines.push_back({ m_currLine, position });
					m_currLine = UINT32_MAX;
				}
			}
		}
		return false;
	}

	template <size_t INDEX, bool NoteOn>
	bool parseVocal(unsigned char pitch, uint32_t position)
	{
		if (m_vocal != UINT32_MAX && m_lyric == m_vocal)
		{
			if constexpr (INDEX == 0)
			{
				if (m_currLine > m_vocal)
					goto SetVocalPosition;
			}
			else
			{
				size_t i = 0;
				while (i < m_lyriclines.size() && m_lyriclines[i].second < position)
					++i;

				if (i >= m_lyriclines.size() || m_lyriclines[i].first > m_vocal)
					goto SetVocalPosition;
			}
			m_values.addSubTrack(INDEX);
			return true;
		}

	SetVocalPosition:
		if constexpr (NoteOn)
			m_vocal = position;
		else
			m_vocal = UINT32_MAX;
		return false;
	}

	void parseText_midi(std::string_view text, uint32_t position)
	{
		if (text[0] != '[')
			m_lyric = position;
	}
};
