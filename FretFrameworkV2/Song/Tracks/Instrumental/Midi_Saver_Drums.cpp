#include "Midi_Saver_Drums.h"

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<4, true>>(MidiFileWriter& writer)
{
	writer.addText(0, "[ENABLE_CHART_DYNAMICS]");
}

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<5, false>>(MidiFileWriter& writer)
{
	writer.addText(0, "[ENABLE_CHART_DYNAMICS]");
}

template <>
void Difficulty_Saver_Midi<DrumNote<4, true>>::write_details(const DifficultyTrack<DrumNote<4, true>>& track, const unsigned char index, MidiFileWriter& writer)
{
	if (!m_details || track.m_notes.isEmpty())
		return;
	m_details = false;

	Midi_Details::Hold flam;
	Midi_Details::Hold toms[3];
	for (const auto& node : track.m_notes)
	{
		if (node->isFlammed())
		{
			if (flam.start == UINT32_MAX)
				flam.start = node.key;
			flam.end = node.key + node->getLongestSustain();
		}
		else if (flam.start != UINT32_MAX)
		{
			if (flam.end > node.key)
				flam.end = node.key;

			writer.addMidiNote(flam.start, 109, 100, flam.end - flam.start);
			flam.start = UINT32_MAX;
		}

		for (char i = 0; i < 3; ++i)
		{
			auto& pad = node->get(i + 1);
			if (!pad.isActive())
				continue;

			if (!pad.isCymbal())
			{
				if (toms[i].start == UINT32_MAX)
					toms[i].start = node.key;
				toms[i].end = node.key + pad.getSustain();
			}
			else if (toms[i].start != UINT32_MAX)
			{
				if (toms[i].end > node.key)
					toms[i].end = node.key;

				writer.addMidiNote(toms[i].start, 110 + i, 100, toms[i].end - toms[i].start);
				toms[i].start = UINT32_MAX;
			}
		}
	}

	if (flam.start != UINT32_MAX)
		writer.addMidiNote(flam.start, 109, 100, flam.end - flam.start);

	for (char i = 0; i < 3; ++i)
		if (toms[i].start != UINT32_MAX)
			writer.addMidiNote(toms[i].start, 110 + i, 100, toms[i].end - toms[i].start);
	m_details = false;
}

template <>
void Difficulty_Saver_Midi<DrumNote<5, false>>::write_details(const DifficultyTrack<DrumNote<5, false>>& track, const unsigned char index, MidiFileWriter& writer)
{
	if (!m_details || track.m_notes.isEmpty())
		return;
	m_details = false;

	Midi_Details::Hold flam;
	for (const auto& node : track.m_notes)
	{
		if (node->isFlammed())
		{
			if (flam.start == UINT32_MAX)
				flam.start = node.key;
			flam.end = node.key + node->getLongestSustain();
		}
		else if (flam.start != UINT32_MAX)
		{
			if (flam.end > node.key)
				flam.end = node.key;

			writer.addMidiNote(flam.start, 109, 100, flam.end - flam.start);
			flam.start = UINT32_MAX;
		}
	}
}
