#pragma once
#include "InstrumentalTrack.h"
#include "Notes/DrumNote_Legacy.h"

template <size_t numPads, bool PRO_DRUMS>
struct Midi_Tracker_Diff<DrumNote<numPads, PRO_DRUMS>>
{
	bool flam = false;
	uint32_t notes[numPads + 1] {};
	constexpr Midi_Tracker_Diff() { for (auto& note : notes) note = UINT32_MAX; }
};

template <size_t numPads, bool PRO_DRUMS>
struct Midi_Tracker_Extensions<DrumNote<numPads, PRO_DRUMS>>
{
	bool enableDynamics = false;
	bool toms[3] = { !PRO_DRUMS, !PRO_DRUMS, !PRO_DRUMS };
};

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalTrack<DrumNote<5, false>>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote<4, true>>::parseLaneColor(Midi_Tracker<DrumNote<4, true>>& tracker, MidiNote note, uint32_t position)
{
	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];
	const int diff = s_diffValues[noteValue];

	if (lane < 5)
	{
		if constexpr (NoteOn)
		{
			DrumNote<4, true>& drums = m_difficulties[diff].get_or_construct_note_midi(position);
			if (tracker.difficulties[diff].flam)
				drums.setFlam(true);

			if (2 <= lane && lane < 5)
				drums.setCymbal(lane, !tracker.ext.toms[lane - 2]);

			if (tracker.ext.enableDynamics)
			{
				if (note.velocity > 100)
					drums.setDynamics(lane, DrumDynamics::Accent);
				else if (note.velocity < 100)
					drums.setDynamics(lane, DrumDynamics::Ghost);
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
void InstrumentalTrack<DrumNote<5, false>>::parseLaneColor(Midi_Tracker<DrumNote<5, false>>& tracker, MidiNote note, uint32_t position)
{
	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];
	const int diff = s_diffValues[noteValue];

	if (lane < 6)
	{
		if constexpr (NoteOn)
		{
			DrumNote<5, false>& drums = m_difficulties[diff].get_or_construct_note_midi(position);
			if (tracker.difficulties[diff].flam)
				drums.setFlam(true);

			if (tracker.ext.enableDynamics)
			{
				if (note.velocity > 100)
					drums.setDynamics(lane, DrumDynamics::Accent);
				else if (note.velocity < 100)
					drums.setDynamics(lane, DrumDynamics::Ghost);
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
void InstrumentalTrack<DrumNote<4, true>>::toggleExtraValues(Midi_Tracker<DrumNote<4, true>>& tracker, MidiNote note, uint32_t position)
{
	if (110 <= note.value && note.value <= 112)
		tracker.ext.toms[note.value - 110] = NoteOn;
}

template <>
void InstrumentalTrack<DrumNote<4, true>>::parseText(Midi_Tracker<DrumNote<4, true>>& tracker, std::string_view str, uint32_t position);

template <>
void InstrumentalTrack<DrumNote<5, false>>::parseText(Midi_Tracker<DrumNote<5, false>>& tracker, std::string_view str, uint32_t position);
