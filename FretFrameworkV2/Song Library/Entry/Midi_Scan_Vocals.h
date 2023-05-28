#pragma once
#include "VocalScan.h"
#include "Serialization/MidiFileReader.h"
namespace Midi_Scanner_Vocal
{
	template<size_t numTracks>
	struct Scanner_Vocal
	{
	public:
		Scanner_Vocal(VocalScan<numTracks>& scan) : m_scan(scan) {}

		template <size_t INDEX = 0>
		void scan(MidiFileReader& reader)
		{
			static_assert(INDEX < numTracks);
			if (m_scan.hasSubTrack(INDEX))
				return;

			while (reader.tryParseEvent())
			{
				MidiEvent midiEvent = reader.getEvent();
				if (midiEvent.type == MidiEventType::Note_On || midiEvent.type == MidiEventType::Note_Off)
				{
					m_position = midiEvent.position;
					MidiNote note = reader.extractMidiNote();
					if (midiEvent.type == MidiEventType::Note_On && note.velocity > 0)
						parseNote<INDEX, true>(note.value);
					else if (parseNote<INDEX, false>(note.value))
					{
						if constexpr (numTracks == 1 || INDEX > 0)
							return;
					}
				}
				else if (midiEvent.type <= MidiEventType::Text_EnumLimit && !m_scan.hasSubTrack(INDEX))
					parseText_midi(reader.extractTextOrSysEx());
			}
		}

	private:
		template <size_t INDEX, bool NoteOn>
		bool parseNote(unsigned char midiValue)
		{
			static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 84 };
			if (PITCHRANGE.first <= midiValue && midiValue <= PITCHRANGE.second)
			{
				if (!m_scan.hasSubTrack(INDEX))
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
			if (m_vocal != UINT64_MAX && m_lyric != UINT64_MAX)
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
				m_scan.addSubTrack(INDEX);
				return true;
			}

		SetVocalPosition:
			if constexpr (NoteOn)
			{
				m_vocal = m_position;
				if (m_lyric != UINT64_MAX)
					m_lyric = m_position;
			}
			else
				m_vocal = UINT64_MAX;
			return false;
		}

		void parseText_midi(std::string_view text)
		{
			if (text[0] != '[')
				m_lyric = m_vocal != UINT64_MAX ? m_vocal : m_position;
		}

	private:
		uint64_t m_position = 0;
		uint64_t m_vocal = UINT64_MAX;
		uint64_t m_lyric = UINT64_MAX;

		uint64_t m_currLine = UINT64_MAX;
		std::vector<std::pair<uint64_t, uint64_t>> m_lyriclines;

		VocalScan<numTracks>& m_scan;
	};

	
}
