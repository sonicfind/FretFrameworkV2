#include "Midi_Saver_Drums.h"

template <>
SimpleFlatMap<std::vector<std::u32string>> Midi_Saver_Instrument::GetStartingEvent<DrumNote<DrumPad_Pro, 4>>() noexcept
{
	SimpleFlatMap<std::vector<std::u32string>> events;
	events.emplace_back(0, { U"[ENABLE_CHART_DYNAMICS]" });
	return events;
}

template <>
SimpleFlatMap<std::vector<std::u32string>> Midi_Saver_Instrument::GetStartingEvent<DrumNote<DrumPad, 5>>() noexcept
{
	SimpleFlatMap<std::vector<std::u32string>> events;
	events.emplace_back(0, { U"[ENABLE_CHART_DYNAMICS]" });
	return events;
}

template <>
bool Difficulty_Saver_Midi::Get_Details<DrumNote<DrumPad_Pro, 4>>(const DifficultyTrack<DrumNote<DrumPad_Pro, 4>>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff)
{
	Midi_Details::Hold flam;
	Midi_Details::Hold toms[3];
	for (const auto& note : track.m_notes)
	{
		if (note->isFlammed())
		{
			if (flam.start == UINT64_MAX)
				flam.start = note.key;
			flam.end = note.key + note->getLongestSustain();
		}
		else if (flam.start != UINT64_MAX)
		{
			if (flam.end > note.key)
				flam.end = note.key;
			notes[flam.start].push_back({ { 0, 109, 100 }, flam.end });
			flam.start = UINT64_MAX;
		}

		for (unsigned char i = 0; i < 3; ++i)
		{
			auto& pad = note->get(i + 1);
			if (!pad.isActive())
				continue;

			if (!pad.isCymbal())
			{
				if (toms[i].start == UINT64_MAX)
					toms[i].start = note.key;
				toms[i].end = note.key + pad.getLength();
			}
			else if (toms[i].start != UINT64_MAX)
			{
				if (toms[i].end > note.key)
					toms[i].end = note.key;

				notes[toms[i].start].push_back({ { 0, unsigned char(110 + i), 100 }, toms[i].end });
				toms[i].start = UINT64_MAX;
			}
		}
	}

	if (flam.start != UINT64_MAX)
		notes[flam.start].push_back({ { 0, 109, 100 }, flam.end });

	for (char i = 0; i < 3; ++i)
		if (toms[i].start != UINT64_MAX)
			notes[toms[i].start].push_back({ { 0, unsigned char(110 + i), 100 }, toms[i].end });
	return track.m_notes.isEmpty();
}

template <>
bool Difficulty_Saver_Midi::Get_Details<DrumNote<DrumPad, 5>>(const DifficultyTrack<DrumNote<DrumPad, 5>>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff)
{
	Midi_Details::Hold flam;
	for (const auto& node : track.m_notes)
	{
		if (node->isFlammed())
		{
			if (flam.start == UINT64_MAX)
				flam.start = node.key;
			flam.end = node.key + node->getLongestSustain();
		}
		else if (flam.start != UINT64_MAX)
		{
			if (flam.end > node.key)
				flam.end = node.key;

			notes[flam.start].push_back({ { 0, 109, 100 }, flam.end });
			flam.start = UINT64_MAX;
		}
	}
	return track.m_notes.isEmpty();
}
