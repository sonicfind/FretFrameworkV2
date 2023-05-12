#pragma once
#include "InstrumentScan.h"
#include "Serialization/MidiFileReader.h"

namespace Midi_Scanner_Instrument
{
	template <class T>
	struct Scanner_Extensions {};

	template <class T>
	class Scanner
	{
	public:
		Scanner(InstrumentScan<T>& scan) : m_scan(scan)
		{
			memcpy(m_laneValues, s_defaultLanes, sizeof(s_defaultLanes));
		}

		template <bool NoteOn>
		void parseNote(MidiNote note)
		{
			if (processSpecialNote<NoteOn>(note))
				return;

			if (s_noteRange.first <= note.value && note.value <= s_noteRange.second)
				parseLaneColor<NoteOn>(note);
			else
				processExtraValues(note);
		}

		void parseText(std::string_view str) {}

	private:
		template <bool NoteOn>
		bool processSpecialNote(MidiNote note) { return false; }

		template <bool NoteOn>
		void parseLaneColor(MidiNote note)
		{
			const int noteValue = note.value - s_noteRange.first;
			const int diff = s_diffValues[noteValue];

			if (m_difficulties[diff].active)
				return;

			const int lane = m_laneValues[noteValue];
			if (lane < T::GetLaneCount())
			{
				if constexpr (!NoteOn)
				{
					if (m_difficulties[diff].notes[lane])
					{
						m_scan.addDifficulty(diff);
						m_difficulties[diff].active = true;
					}
				}
				else
					m_difficulties[diff].notes[lane] = true;
			}
		}

		void processExtraValues(MidiNote note) { }
		

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
		Scanner_Extensions<T> m_ext;
		InstrumentScan<T>& m_scan;
	};

	template <class T>
	bool Scan(InstrumentScan<T>& scan, MidiFileReader& reader)
	{
		if (scan.getSubTracks() > 0)
			return false;

		Scanner<T> scanner(scan);
		while (const auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					scanner.parseNote<true>(note);
				else
					scanner.parseNote<false>(note);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				scanner.parseNote<false>(reader.extractMidiNote());
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				scanner.parseText(reader.extractTextOrSysEx());

			if (scan.isComplete())
				break;
		}
		return true;
	}
}

