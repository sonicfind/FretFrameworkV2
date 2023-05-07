#pragma once
#include "ScanValues.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"
#include "Serialization/MidiFileReader.h"

namespace VocalScan_Midi
{
	template<size_t numTracks>
	struct Midi_Scanner_Vocal
	{
		uint32_t m_vocal = UINT32_MAX;
		uint32_t m_lyric = UINT32_MAX;

		uint32_t m_currLine = UINT32_MAX;
		std::vector<std::pair<uint32_t, uint32_t>> m_lyriclines;

		template <size_t INDEX, bool NoteOn>
		bool parseNote(ScanValues& values, unsigned char midiValue, uint32_t position)
		{
			static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
			if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
			{
				if (!values.wasTrackValidated(INDEX))
					return parseVocal<INDEX, NoteOn>(values, midiValue, position);
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
		bool parseVocal(ScanValues& values, unsigned char pitch, uint32_t position)
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
				values.addSubTrack(INDEX);
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

	template <size_t INDEX, size_t numTracks>
	void Scan(ScanValues& values, Midi_Scanner_Vocal<numTracks>& tracker, MidiFileReader& reader)
	{
		static_assert(INDEX < numTracks);
		if (values.wasTrackValidated(INDEX))
			return;

		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On || midiEvent->type == MidiEventType::Note_Off)
			{
				MidiNote note = reader.extractMidiNote();
				if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
					tracker.parseNote<INDEX, true>(values, note.value, midiEvent->position);
				else if (tracker.parseNote<INDEX, false>(values, note.value, midiEvent->position))
				{
					if constexpr (numTracks == 1 || INDEX > 0)
						return;
				}
			}
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit && values.wasTrackValidated(INDEX))
				tracker.parseText_midi(reader.extractTextOrSysEx(), midiEvent->position);
		}
	}
}
