#pragma once
#include "InstrumentScan.h"
#include "Serialization/MidiFileReader.h"

template <class T>
struct Midi_Scanner_Extensions {};

template <class T>
class Midi_Scanner
{
public:
	Midi_Scanner(ScanValues& values, MidiFileReader& reader) : m_values(values)
	{
		memcpy(m_laneValues, s_defaultLanes, sizeof(s_defaultLanes));
		while (const auto midiEvent = reader.parseEvent())
		{
			bool found = false;
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					found = parseNote<true>(note);
				else
					found = parseNote<false>(note);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				found = parseNote<false>(reader.extractMidiNote());
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText(reader.extractTextOrSysEx());

			if (found && isFinished())
				break;
		}
	}

public:
	Midi_Scanner_Extensions<T> m_ext;

private:
	bool isFinished() const noexcept
	{
		return m_values.m_subTracks == 15;
	}

	template <bool NoteOn>
	bool parseNote(MidiNote note)
	{
		if (processSpecialNote<NoteOn>(note))
			return true;

		if (s_noteRange.first <= note.value && note.value <= s_noteRange.second)
			return parseLaneColor<NoteOn>(note);
		else
			return processExtraValues(note);
	}

	template <bool NoteOn>
	bool processSpecialNote(MidiNote note) { return false; }

	template <bool NoteOn>
	bool parseLaneColor(MidiNote note)
	{
		const int noteValue = note.value - s_noteRange.first;
		const int diff = s_diffValues[noteValue];

		if (m_difficulties[diff].active)
			return false;
		
		const int lane = m_laneValues[noteValue];
		if (lane < T::GetLaneCount())
		{
			if constexpr (!NoteOn)
			{
				if (m_difficulties[diff].notes[lane])
				{
					m_values.addSubTrack(diff);
					m_difficulties[diff].active = true;
					return true;
				}
			}
			else
				m_difficulties[diff].notes[lane] = true;
		}
		return false;
	}

	bool processExtraValues(MidiNote note) { return false; }
	void parseText(std::string_view str) {}

private:
	static constexpr int s_defaultLanes[48] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	};

	static constexpr int s_diffValues[48] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
	};

	static constexpr std::pair<unsigned char, unsigned char> s_noteRange{ 60, 100 };

	int m_laneValues[48];

	struct
	{
		bool notes[T::GetLaneCount()]{};
		bool active = false;
	} m_difficulties[4];
	ScanValues& m_values;
};
