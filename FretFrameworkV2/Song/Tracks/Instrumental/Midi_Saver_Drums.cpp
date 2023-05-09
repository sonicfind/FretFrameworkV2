#include "Midi_Saver_Drums.h"

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<DrumPad_Pro, 4>>(MidiFileWriter& writer)
{
	writer.addText(0, "[ENABLE_CHART_DYNAMICS]");
}

template <>
void Midi_Saver_Instrument::WriteMidiToggleEvent<DrumNote<DrumPad, 5>>(MidiFileWriter& writer)
{
	writer.addText(0, "[ENABLE_CHART_DYNAMICS]");
}

template <>
void Difficulty_Saver_Midi<DrumNote<DrumPad_Pro, 4>>::write_details(const DifficultyTrack<DrumNote<DrumPad_Pro, 4>>& track, const unsigned char index, MidiFileWriter& writer)
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
				toms[i].end = node.key + pad.getLength();
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
void Difficulty_Saver_Midi<DrumNote<DrumPad, 5>>::write_details(const DifficultyTrack<DrumNote<DrumPad, 5>>& track, const unsigned char index, MidiFileWriter& writer)
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
