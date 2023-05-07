#pragma once
#include "InstrumentalScan.h"
#include "Serialization/MidiFileReader.h"

namespace InstrumentalScan_Midi
{
	template <class T>
	struct Midi_Scanner_Extensions {};

	template <class T>
	struct Midi_Scanner
	{
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

		Midi_Scanner_Extensions<T> m_ext;
		Midi_Scanner()
		{
			memcpy(m_laneValues, s_defaultLanes, sizeof(s_defaultLanes));
		}

		template <bool NoteOn>
		bool parseLaneColor(ScanValues& values, MidiNote note)
		{
			const int noteValue = note.value - s_noteRange.first;
			const int diff = s_diffValues[noteValue];

			if (!m_difficulties[diff].active)
			{
				const int lane = m_laneValues[noteValue];
				if (lane < T::GetLaneCount())
				{
					if constexpr (!NoteOn)
					{
						if (m_difficulties[diff].notes[lane])
						{
							values.addSubTrack(diff);
							m_difficulties[diff].active = true;
							return values.m_subTracks == 15;
						}
					}
					else
						m_difficulties[diff].notes[lane] = true;
				}
			}
			return false;
		}

		bool processExtraValues(ScanValues& values, MidiNote note) { return false; }
		void parseText(std::string_view str) {}
	};

	template <class T>
	Midi_Scanner<T> Scan(ScanValues& values, MidiFileReader& reader)
	{
		Midi_Scanner<T> scanner;
		while (const auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On || midiEvent->type == MidiEventType::Note_Off)
			{
				MidiNote note = reader.extractMidiNote();
				if (Midi_Scanner<T>::s_noteRange.first <= note.value && note.value <= Midi_Scanner<T>::s_noteRange.second)
				{
					if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
						scanner.parseLaneColor<true>(values, note);
					else if (scanner.parseLaneColor<false>(values, note))
						break;
				}
				else if (scanner.processExtraValues(values, note))
					break;
			}
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				scanner.parseText(reader.extractTextOrSysEx());
		}
		return scanner;
	}
};
