#pragma once
#include "ScanValues.h"
#include "Serialization/MidiFileReader.h"

template<size_t numTracks>
struct Midi_Scanner_Vocal
{
public:
	Midi_Scanner_Vocal(ScanValues& values) : m_values(values) {}

	template <size_t INDEX = 0>
	void scan(MidiFileReader& reader)
	{
		static_assert(INDEX < numTracks);
		if (m_values.wasTrackValidated(INDEX))
			return;

		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On || midiEvent->type == MidiEventType::Note_Off)
			{
				m_position = midiEvent->position;
				MidiNote note = reader.extractMidiNote();
				if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
					parseNote<INDEX, true>(note.value);
				else if (parseNote<INDEX, false>(note.value))
				{
					if constexpr (numTracks == 1 || INDEX > 0)
						return;
				}
			}
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit && m_values.wasTrackValidated(INDEX))
				parseText_midi(reader.extractTextOrSysEx());
		}
	}

private:
	template <size_t INDEX, bool NoteOn>
	bool parseNote(unsigned char midiValue)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
		if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
		{
			if (!m_values.wasTrackValidated(INDEX))
				return parseVocal<INDEX, NoteOn>(midiValue);
		}
		else if constexpr (INDEX == 0)
		{
			if (midiValue == 105 || midiValue == 106)
			{
				if constexpr (NoteOn)
					m_currLine = m_position;
				else if (m_currLine != UINT64_MAX)
				{
					if constexpr (numTracks > 1)
						m_lyriclines.push_back({ m_currLine, m_position });
					m_currLine = UINT64_MAX;
				}
			}
		}
		return false;
	}

	template <size_t INDEX, bool NoteOn>
	bool parseVocal(unsigned char pitch)
	{
		if (m_vocal != UINT64_MAX && m_lyric == m_vocal)
		{
			if constexpr (INDEX == 0)
			{
				if (m_currLine > m_vocal)
					goto SetVocalPosition;
			}
			else
			{
				size_t i = 0;
				while (i < m_lyriclines.size() && m_lyriclines[i].second < m_position)
					++i;

				if (i >= m_lyriclines.size() || m_lyriclines[i].first > m_vocal)
					goto SetVocalPosition;
			}
			m_values.addSubTrack(INDEX);
			return true;
		}

	SetVocalPosition:
		if constexpr (NoteOn)
			m_vocal = m_position;
		else
			m_vocal = UINT64_MAX;
		return false;
	}

	void parseText_midi(std::string_view text)
	{
		if (text[0] != '[')
			m_lyric = m_position;
	}

private:
	uint64_t m_position = 0;
	uint64_t m_vocal = UINT64_MAX;
	uint64_t m_lyric = UINT64_MAX;

	uint64_t m_currLine = UINT64_MAX;
	std::vector<std::pair<uint64_t, uint64_t>> m_lyriclines;

	ScanValues& m_values;
};
