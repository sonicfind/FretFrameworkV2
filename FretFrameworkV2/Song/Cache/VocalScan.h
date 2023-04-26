#pragma once
#include "ScanTrack.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"
#include "Serialization/MidiFileReader.h"
#include <assert.h>

template <size_t numTracks>
class VocalScan : public ScanTrack, public BCH_CHT_Scannable
{
	struct Midi_Scanner_Vocal
	{
		uint32_t vocal = UINT32_MAX;
		uint32_t lyric = UINT32_MAX;

		uint32_t currLine = UINT32_MAX;
		std::vector<std::pair<uint32_t, uint32_t>>& lyriclines;
		Midi_Scanner_Vocal(std::vector<std::pair<uint32_t, uint32_t>>& lines) : lyriclines(lines) {}
	};

public:
	template <size_t INDEX = 0>
	void scan(MidiFileReader& reader, std::vector<std::pair<uint32_t, uint32_t>>& lines)
	{
		static_assert(INDEX < numTracks);
		if (((1 << INDEX) & m_subTracks) > 0)
			return;

		Midi_Scanner_Vocal scanner(lines);
		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On || midiEvent->type == MidiEventType::Note_Off)
			{
				MidiNote note = reader.extractMidiNote();
				if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
					parseNote<INDEX, true>(scanner, note.value, midiEvent->position);
				else if (parseNote<INDEX, false>(scanner, note.value, midiEvent->position))
				{
					if constexpr (numTracks == 1 || INDEX > 0)
						return;
				}
			}
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit && ((1 << INDEX) & m_subTracks) == 0)
				parseText_midi(scanner, reader.extractTextOrSysEx(), midiEvent->position);
		}
	}

public:
	virtual void scan(CommonChartParser* parser) override
	{
		if (m_subTracks > 0)
			return;

		VocalPitch pitch;
		uint32_t endOfPhrase = 0;
		parser->nextEvent();
		while (parser->isStillCurrentTrack())
		{
			const auto trackEvent = parser->parseEvent();
			if (trackEvent.second == ChartEvent::VOCAL)
			{
				if (trackEvent.first >= endOfPhrase)
					break;

				auto lyric = parser->extractLyric();
				if (lyric.first == 0 || lyric.first > numTracks || ((1 << lyric.first) & m_subTracks) > 0)
					break;

				auto values = parser->extractPitchAndDuration();
				if (pitch.set(values.first))
				{
					m_subTracks |= 1 << lyric.first;
					if (m_subTracks == (1 << numTracks) - 1)
						return;
				}
				break;
			}
			else if (trackEvent.second == ChartEvent::SPECIAL)
			{
				auto phrase = parser->extractSpecialPhrase();
				if (phrase.getType() == SpecialPhraseType::LyricLine)
					endOfPhrase = trackEvent.first + phrase.getDuration();
				break;
			}
			parser->nextEvent();
		}
	}

private:
	template <size_t INDEX, bool NoteOn>
	bool parseNote(Midi_Scanner_Vocal& scanner, unsigned char midiValue, uint32_t position)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
		if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
		{
			if (((1 << INDEX) & m_subTracks) == 0)
				return parseVocal<INDEX, NoteOn>(scanner, midiValue, position);
		}
		else if constexpr (INDEX == 0)
		{
			if (midiValue == 105 || midiValue == 106)
			{
				if constexpr (NoteOn)
					scanner.currLine = position;
				else if (scanner.currLine != UINT32_MAX)
				{
					if constexpr (numTracks > 1)
						scanner.lyriclines.push_back({ scanner.currLine, position });
					scanner.currLine = UINT32_MAX;
				}
			}
		}
		return false;
	}

	template <size_t INDEX, bool NoteOn>
	bool parseVocal(Midi_Scanner_Vocal& scanner, unsigned char pitch, uint32_t position)
	{
		if (scanner.vocal != UINT32_MAX && scanner.lyric == scanner.vocal)
		{
			if constexpr (INDEX == 0)
			{
				if (scanner.currLine > scanner.vocal)
					goto SetVocalPosition;
			}
			else
			{
				size_t i = 0;
				while (i < scanner.lyriclines.size() && scanner.lyriclines[i].second < position)
					++i;

				if (i >= scanner.lyriclines.size() || scanner.lyriclines[i].first > scanner.vocal)
					goto SetVocalPosition;
			}
			m_subTracks |= 1 << INDEX;
			return true;
		}

SetVocalPosition:
		if constexpr (NoteOn)
			scanner.vocal = position;
		else
			scanner.vocal = UINT32_MAX;
		return false;
	}

	void parseText_midi(Midi_Scanner_Vocal& scanner, std::string_view text, uint32_t position)
	{
		if (text[0] != '[')
			scanner.lyric = position;
	}
};
