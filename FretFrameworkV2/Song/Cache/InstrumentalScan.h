#pragma once
#include "ScanTrack.h"
#include "Serialization/TxtFileReader.h"
#include "Serialization/MidiFileReader.h"

template <class T>
class InstrumentalScan : public ScanTrack
{
public:
	InstrumentalScan() = default;
	InstrumentalScan(InstrumentalScan&&) = default;
	InstrumentalScan& operator=(InstrumentalScan&&) = default;

	InstrumentalScan(const InstrumentalScan&) = delete;
	InstrumentalScan& operator=(const InstrumentalScan&) = delete;

	void scan_V1(size_t diff, TxtFileReader& reader)
	{
		if ((1 << diff) & m_subTracks)
			return;

		bool extraTestDone = false;
		while (reader.isStillCurrentTrack())
		{
			const auto chartEvent = reader.parseEvent();
			if (chartEvent.second == ChartEvent::NOTE)
			{
				auto note = reader.extractColorAndSustain_V1();
				if (T::TestIndex_V1(note.first))
					m_subTracks |= 1 << diff;

				if (!extraTestDone)
					extraTestDone = extraTest_V1(note.first);

				if (((1 << diff) & m_subTracks) > 0 && extraTestDone)
				{
					reader.skipTrack();
					return;
				}
			}
			reader.nextEvent();
		}
	}

protected:
	[[nodiscard]] virtual bool extraTest_V1(size_t note)
	{
		return true;
	}

public:
	struct Midi_Scanner_Extensions {};
	struct Midi_Scanner
	{
		static constexpr int s_defaultLanes[48] =
		{
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		};

		static constexpr bool s_doExtra = false;

		int laneValues[48];

		struct
		{
			bool notes[T::GetLaneCount()]{};
			bool active = false;
		} difficulties[4];

		bool extraTested = !s_doExtra;
		Midi_Scanner_Extensions ext;
		Midi_Scanner()
		{
			memcpy(laneValues, s_defaultLanes, sizeof(s_defaultLanes));
		}
	};

	void scan(MidiFileReader& reader)
	{
		if (m_subTracks > 0)
			return;

		Midi_Scanner scanner;
		while (const auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On || midiEvent->type == MidiEventType::Note_Off)
			{
				MidiNote note = reader.extractMidiNote();
				if (s_noteRange.first <= note.value && note.value <= s_noteRange.second)
				{
					if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
						parseLaneColor<true>(scanner, note);
					else
					{
						parseLaneColor<false>(scanner, note);
						if (m_subTracks == 15)
							return;
					}
				}
				else if (!scanner.extraTested)
				{
					if (midiEvent->type == MidiEventType::Note_On && note.velocity > 0)
						toggleExtraValues<true>(scanner, note);
					else
						toggleExtraValues<false>(scanner, note);
				}
			}
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText(scanner, reader.extractTextOrSysEx());
		}
	}

protected:
	static constexpr std::pair<unsigned char, unsigned char> s_noteRange{ 60, 100 };
	
	static constexpr int s_diffValues[48] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
	};

	template <bool NoteOn>
	void parseLaneColor(Midi_Scanner& scanner, MidiNote note)
	{
		const int noteValue = note.value - s_noteRange.first;
		const int diff = s_diffValues[noteValue];

		if (!scanner.difficulties[diff].active)
		{
			const int lane = scanner.laneValues[noteValue];
			if (lane < T::GetLaneCount())
			{
				if constexpr (!NoteOn)
				{
					if (scanner.difficulties[diff].notes[lane])
					{
						m_subTracks |= 1 << diff;
						scanner.difficulties[diff].active = true;
					}
				}
				scanner.difficulties[diff].notes[lane] = true;
			}
		}
	}

	template <bool NoteOn>
	void toggleExtraValues(Midi_Scanner& scanner, MidiNote note) {}
	virtual void applyExtraValue(Midi_Scanner& scanner)
	{
		scanner.extraTested = true;
	}
	void parseText(Midi_Scanner& scanner, std::string_view str) {}
};

template <class T>
class InstrumentalScan_Extended : public InstrumentalScan<T>, public BCH_CHT_Scannable
{
public:
	using InstrumentalScan<T>::InstrumentalScan;
	using InstrumentalScan<T>::scan;
	virtual void scan(CommonChartParser& parser) override
	{
		while (parser.isStillCurrentTrack())
		{
			if (!parser.isStartOfTrack())
				parser.nextEvent();
			else if (parser.validateDifficultyTrack())
			{
				const size_t diff = parser.getDifficulty();
				if (diff < 5 && ((1 << diff) & this->m_subTracks) == 0)
					scanDifficulty(diff, parser);
				else // BCH only
					parser.skipTrack();
			}
			else
				parser.skipTrack();
		}
	}

private:
	void scanDifficulty(size_t diff, CommonChartParser& parser)
	{
		while (parser.isStillCurrentTrack())
		{
			const auto trackEvent = parser.parseEvent();
			bool add = false;
			if (trackEvent.second == ChartEvent::NOTE)
			{
				auto color = parser.extractSingleNote();
				add = T::TestIndex(color.first);
			}
			else if (trackEvent.second == ChartEvent::MULTI_NOTE)
			{
				const auto colors = parser.extractMultiNote();
				if (!colors.empty())
				{
					for (const auto& color : colors)
					{
						if (!T::TestIndex(color.first))
						{
							add = false;
							break;
						}
					}
				}
			}

			if (add)
			{
				this->m_subTracks |= 1 << diff;
				parser.skipTrack();
				return;
			}
			parser.nextEvent();
		}
	}
};
