#pragma once
#include "InstrumentScan.h"
#include "Serialization/MidiFileReader.h"

namespace Midi_Scanner_Instrument
{
	template <class T>
	struct Scanner_Lanes
	{
		size_t values[96];
		Scanner_Lanes() : values{
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		} {}
	};

	template <class T>
	struct Scanner_Diff
	{
		static constexpr std::pair<unsigned char, unsigned char> NOTERANGE{ 60, 100 };
		bool notes[T::GetLaneCount()]{};
		bool active = false;
	};

	template <class T>
	struct Scanner_Extensions {};

	template <class T>
	class Scanner
	{
	public:
		Scanner(InstrumentScan<T>& scan) : m_scan(scan) {}

		template <bool NoteOn>
		void parseNote(MidiNote note, unsigned char channel)
		{
			if (processSpecialNote<NoteOn>(note))
				return;

			if (Scanner_Diff<T>::NOTERANGE.first <= note.value && note.value <= Scanner_Diff<T>::NOTERANGE.second)
				parseLaneColor<NoteOn>(note, channel);
			else
				processExtraValues(note);
		}

		void parseText(std::string_view str) {}

	private:
		size_t getDifficulty(size_t noteValue) const noexcept {
			static constexpr size_t DIFFS[48] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
			};
			return DIFFS[noteValue];
		}

		template <bool NoteOn>
		bool processSpecialNote(MidiNote note) { return false; }

		template <bool NoteOn>
		void parseLaneColor(MidiNote note, unsigned char channel)
		{
			const int noteValue = note.value - Scanner_Diff<T>::NOTERANGE.first;
			const size_t diff = getDifficulty(noteValue);

			if (m_difficulties[diff].active)
				return;

			const size_t lane = m_lanes.values[noteValue];
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
		Scanner_Lanes<T> m_lanes;
		Scanner_Diff<T> m_difficulties[4]{};
		Scanner_Extensions<T> m_ext;
		InstrumentScan<T>& m_scan;
	};

	template <class T>
	bool Scan(InstrumentScan<T>& scan, MidiFileReader& reader)
	{
		if (scan.getSubTracks() > 0)
			return false;

		Scanner<T> scanner(scan);
		while (reader.tryParseEvent())
		{
			MidiEvent midiEvent = reader.getEvent();
			if (midiEvent.type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					scanner.parseNote<true>(note, midiEvent.channel);
				else
					scanner.parseNote<false>(note, midiEvent.channel);
			}
			else if (midiEvent.type == MidiEventType::Note_Off)
				scanner.parseNote<false>(reader.extractMidiNote(), midiEvent.channel);
			else if (midiEvent.type <= MidiEventType::Text_EnumLimit)
				scanner.parseText(reader.extractTextOrSysEx());

			if (scan.isComplete())
				break;
		}
		return true;
	}
}


