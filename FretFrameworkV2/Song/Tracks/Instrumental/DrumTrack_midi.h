#pragma once
#include "InstrumentalTrack.h"
#include "Notes/DrumNote.h"
#include "Midi_Hold.h"

template <>
struct InstrumentalTrack<DrumNote<4, true>, true>::Midi_Tracker_Diff
{
	bool flam = false;
	uint32_t notes[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
struct InstrumentalTrack<DrumNote<5, false>, true>::Midi_Tracker_Diff
{
	bool flam = false;
	uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
struct InstrumentalTrack<DrumNote<4, true>, true>::Midi_Tracker_Extensions
{
	bool enableDynamics = false;
	bool toms[3] = { false, false, false };
};

template <>
struct InstrumentalTrack<DrumNote<5, false>, true>::Midi_Tracker_Extensions
{
	bool enableDynamics = false;
};

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalTrack<DrumNote<5, false>, true>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote<4, true>, true>::parseLaneColor(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];

	if (lane < 5)
	{
		const int diff = s_diffValues[noteValue];
		if constexpr (NoteOn)
		{
			DrumNote<4, true>& drums = m_difficulties[diff].get_or_construct_note_midi(position);
			if (tracker.difficulties[diff].flam)
				drums.setFlam(true);

			if (lane > 0)
			{
				auto& pad = drums.get(lane - 1);
				if (2 <= lane && lane < 5)
					pad.setCymbal(!tracker.ext.toms[lane - 2]);

				if (tracker.ext.enableDynamics)
				{
					if (note.velocity > 100)
						pad.setDynamics(DrumDynamics::Accent);
					else if (note.velocity < 100)
						pad.setDynamics(DrumDynamics::Ghost);
				}
			}
			tracker.difficulties[diff].notes[lane] = position;
		}
		else
		{
			uint32_t colorPosition = tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_difficulties[diff].m_notes.getNodeFromBack(colorPosition).set(lane, position - colorPosition);
				tracker.difficulties[diff].notes[lane] = UINT32_MAX;
			}
		}
	}
	else if (note.value == 95)
	{
		if constexpr (NoteOn)
		{
			tracker.difficulties[3].notes[0] = position;
			m_difficulties[3].get_or_construct_note_midi(position).modify('+');
		}
		else
		{
			uint32_t colorPosition = tracker.difficulties[3].notes[0];
			if (colorPosition != UINT32_MAX)
			{
				m_difficulties[3].m_notes.getNodeFromBack(colorPosition).set(0, position - colorPosition);
				tracker.difficulties[3].notes[0] = UINT32_MAX;
			}
		}
	}
}

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote<5, false>, true>::parseLaneColor(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];

	if (lane < 6)
	{
		const int diff = s_diffValues[noteValue];
		if constexpr (NoteOn)
		{
			DrumNote<5, false>& drums = m_difficulties[diff].get_or_construct_note_midi(position);
			if (tracker.difficulties[diff].flam)
				drums.setFlam(true);

			if (tracker.ext.enableDynamics && lane > 0)
			{
				auto& pad = drums.get(lane - 1);
				if (note.velocity > 100)
					pad.setDynamics(DrumDynamics::Accent);
				else if (note.velocity < 100)
					pad.setDynamics(DrumDynamics::Ghost);
			}

			tracker.difficulties[diff].notes[lane] = position;
		}
		else
		{
			uint32_t colorPosition = tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_difficulties[diff].m_notes.getNodeFromBack(colorPosition).set(lane, position - colorPosition);
				tracker.difficulties[diff].notes[lane] = UINT32_MAX;
			}
		}
	}
	else if (note.value == 95)
	{
		if constexpr (NoteOn)
		{
			tracker.difficulties[3].notes[0] = position;
			m_difficulties[3].get_or_construct_note_midi(position).modify('+');
		}
		else
		{
			uint32_t colorPosition = tracker.difficulties[3].notes[0];
			if (colorPosition != UINT32_MAX)
			{
				m_difficulties[3].m_notes.getNodeFromBack(colorPosition).set(0, position - colorPosition);
				tracker.difficulties[3].notes[0] = UINT32_MAX;
			}
		}
	}
}

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote<4, true>, true>::toggleExtraValues(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	if (note.value == 109)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			tracker.difficulties[i].flam = NoteOn;
			if constexpr (NoteOn)
				if (auto drum = m_difficulties[i].m_notes.try_back(position))
					drum->setFlam(true);
		}
	}
	else if (110 <= note.value && note.value <= 112)
		tracker.ext.toms[note.value - 110] = NoteOn;
}

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote<5, false>, true>::toggleExtraValues(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	if (note.value == 109)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			tracker.difficulties[i].flam = NoteOn;
			if constexpr (NoteOn)
				if (auto drum = m_difficulties[i].m_notes.try_back(position))
					drum->setFlam(true);
		}
	}
}

template <>
void InstrumentalTrack<DrumNote<4, true>, true>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position);

template <>
void InstrumentalTrack<DrumNote<5, false>, true>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position);

template <>
void InstrumentalTrack<DrumNote<4, true>, true>::writeMidiToggleEvent(MidiFileWriter& writer) const;

template <>
void InstrumentalTrack<DrumNote<5, false>, true>::writeMidiToggleEvent(MidiFileWriter& writer) const;

template <>
template <unsigned char INDEX>
void DifficultyTrack<DrumNote<4, true>, false>::write_details(MidiFileWriter& writer, bool& doDetails) const
{
	if (!doDetails || m_notes.isEmpty())
		return;

	Midi_Details::Hold flam;
	Midi_Details::Hold toms[3];
	for (const auto& node : m_notes)
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

	doDetails = false;
}

template <>
template <unsigned char INDEX>
void DifficultyTrack<DrumNote<5, false>, false>::write_details(MidiFileWriter& writer, bool& doDetails) const
{
	if (!doDetails || m_notes.isEmpty())
		return;

	Midi_Details::Hold flam;
	for (const auto& node : m_notes)
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
	doDetails = false;
}
