#pragma once
#include "InstrumentalTrack.h"
#include "File Processing/MidiFileReader.h"

namespace
{
	template <class T>
	struct Tracker
	{
		struct
		{
			uint32_t notes[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		} difficulties[5];
	};
}

template <class T>
class InstrumentTrackMidiParser
{
	using ValCombo = std::pair<SpecialPhraseType, uint32_t>;
public:
	InstrumentTrackMidiParser(InstrumentalTrack<T>& track, MidiFileReader& reader)
		: m_track(track), m_reader(reader)
	{
		init();
		loadFromMidi();
	}

private:
	void init() {}

	void loadFromMidi()
	{
		if (m_track.isOccupied())
			throw std::runtime_error("Track defined multiple times in file");

		while (m_reader.parseEvent())
		{
			MidiEventType eventType = m_reader.getEventType();
			if (eventType == MidiEventType::Note_On)
				parseNote<true>(m_reader.extractMidiNote());
			else if (eventType == MidiEventType::Note_Off)
				parseNote<false>(m_reader.extractMidiNote());
			else if (eventType == MidiEventType::SysEx || eventType == MidiEventType::SysEx_End)
				parseSysEx(m_reader.extractTextOrSysEx());
			else if (eventType <= MidiEventType::Text_EnumLimit)
				parseText(m_reader.extractTextOrSysEx());
		}

		m_track.shrink_midi();
	}

	template <bool NoteOn>
	void parseNote(MidiNote note)
	{
		const bool isON = NoteOn && note.velocity > 0;
		const uint32_t position = m_reader.getPosition();
		if (m_noteRange.first <= note.value && note.value < m_noteRange.second)
			parseLaneColor(note, isON);
		else if (120 <= note.value && note.value <= 124)
			parseBRE(note.value, isON);
		else if (note.value == m_reader.getStarPowerValue())
			addSpecialPhrase(m_starPower, isON);
		else if (note.value == 103)
			addSpecialPhrase(m_solo, isON);
		else if (note.value == 126)
			addSpecialPhrase(m_tremolo, isON);
		else if (note.value == 127)
			addSpecialPhrase(m_trill, isON);
		else
			toggleExtraValues(note, isON);
	}

	void parseSysEx(std::string_view sysex)
	{
	}

	void parseText(std::string_view text)
	{
		m_track.get_or_emplace_Events_midi(m_reader.getPosition()).push_back(UnicodeString::strToU32(text));
	}

	void addSpecialPhrase(ValCombo& combo, const bool isON)
	{
		uint32_t position = m_reader.getPosition();
		if (isON)
			combo.second = position;
		else if (combo.second != UINT32_MAX)
		{
			m_track.get_or_emplace_SpecialPhrase_midi(combo.second).push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	void addSpecialPhrase(int diff, ValCombo& combo, const bool isON)
	{
		uint32_t position = m_reader.getPosition();
		if (isON)
			combo.second = position;
		else if (combo.second != UINT32_MAX)
		{
			m_track.get_or_emplace_SpecialPhrase_midi(diff, combo.second).push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	void parseLaneColor(MidiNote note, const bool isON)
	{
		const int noteValue = note.value - m_noteRange.first;
		const int lane = m_laneValues[noteValue];
		const int diff = s_diffValues[noteValue];

		if (lane < 5)
		{
			const uint32_t position = m_reader.getPosition();
			if (isON)
			{
				m_track.construct_note_midi(diff, position);
				m_tracker.difficulties[diff].notes[lane] = position;
			}
			else
			{
				uint32_t& colorPosition = m_tracker.difficulties[diff].notes[lane];
				if (colorPosition != UINT32_MAX)
				{
					m_track.getNote_midi(diff, colorPosition).set(lane + 1, position - colorPosition);
					colorPosition = UINT32_MAX;
				}
			}
		}
	}

	void parseBRE(int noteValue, const bool isON)
	{
		const uint32_t position = m_reader.getPosition();

		if (isON)
		{
			m_tracker.difficulties[4].notes[noteValue - 120] = position;
			m_doBRE = m_tracker.difficulties[4].notes[0] == m_tracker.difficulties[4].notes[1] &&
					    m_tracker.difficulties[4].notes[1] == m_tracker.difficulties[4].notes[2] &&
					    m_tracker.difficulties[4].notes[2] == m_tracker.difficulties[4].notes[3];
		}
		else if (m_doBRE)
		{
			uint32_t colorPosition = m_tracker.difficulties[4].notes[0];
			m_track.addSharedPhrase(colorPosition, { SpecialPhraseType::StarPowerActivation, position - colorPosition });

			for (size_t i = 0; i < 5; ++i)
				m_tracker.difficulties[4].notes[i] = UINT32_MAX;
			m_doBRE = false;
		}
		else
		{
			const int lane = noteValue - 120;
			uint32_t& colorPosition = m_tracker.difficulties[4].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_track.addNote(4, colorPosition, lane + 1, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		} 
	}

	void toggleExtraValues(MidiNote note, const bool isON)
	{
	}

private:
	static constexpr int s_diffValues[48] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
	};

	std::pair<unsigned char, unsigned char> m_noteRange = { 60, 101 };

	int m_laneValues[48] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	};

	InstrumentalTrack<T>& m_track;
	MidiFileReader& m_reader;
	Tracker<T> m_tracker;

	ValCombo m_solo = { SpecialPhraseType::Solo, UINT32_MAX };
	ValCombo m_starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
	ValCombo m_tremolo = { SpecialPhraseType::Tremolo, UINT32_MAX };
	ValCombo m_trill = { SpecialPhraseType::Trill, UINT32_MAX };
	bool m_doBRE = false;
};
