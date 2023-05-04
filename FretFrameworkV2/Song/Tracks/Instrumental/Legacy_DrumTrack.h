#pragma once
#include "InstrumentalTrack.h"
#include "Notes/DrumNote_Legacy.h"

class Legacy_DrumTrack : public InstrumentalTrack<DrumNote_Legacy, false>
{
private:
	using InstrumentalTrack<DrumNote_Legacy, false>::load;

public:
	Legacy_DrumTrack(DrumType_Enum type);
	Legacy_DrumTrack(MidiFileReader& reader);

	void load_V1(size_t diff, ChtFileReader& reader);

	[[nodiscard]] DrumType_Enum getDrumType() const noexcept { return m_type; }

	template <size_t numPads, bool PRO_DRUMS>
	void transfer(InstrumentalTrack_Extended<DrumNote<numPads, PRO_DRUMS>>& track)
	{
		track.m_specialPhrases = std::move(m_specialPhrases);
		track.m_events = std::move(m_events);
		for (size_t i = 0; i < 5; ++i)
		{
			if (!track.m_difficulties[i].isOccupied() && m_difficulties[i].isOccupied())
			{
				track.m_difficulties[i].m_specialPhrases = std::move(m_difficulties[i].m_specialPhrases);
				track.m_difficulties[i].m_events = std::move(m_difficulties[i].m_events);
				track.m_difficulties[i].m_notes.reserve(m_difficulties[i].m_notes.size());
				for (const auto& note : m_difficulties[i].m_notes)
					track.m_difficulties[i].m_notes.emplace_back(note.key, note->transformNote<numPads, PRO_DRUMS>());
				m_difficulties[i].m_notes.clear();
			}
		}
	}

private:
	[[nodiscard]] DrumType_Enum evaluateDrumType(size_t index);
	DrumType_Enum m_type = DrumType_Enum::LEGACY;
};

template <>
struct InstrumentalTrack<DrumNote_Legacy, false>::Midi_Tracker_Diff
{
	bool flam = false;
	uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
struct InstrumentalTrack<DrumNote_Legacy, false>::Midi_Tracker_Extensions
{
	bool enableDynamics = false;
	bool toms[3] = { false, false, false };
};

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalTrack<DrumNote_Legacy, false>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote_Legacy, false>::parseLaneColor(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];
	const int diff = s_diffValues[noteValue];

	if (lane < 6)
	{
		if constexpr (NoteOn)
		{
			DrumNote_Legacy& drums = m_difficulties[diff].get_or_construct_note_midi(position);
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
void InstrumentalTrack<DrumNote_Legacy, false>::toggleExtraValues(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	if (110 <= note.value && note.value <= 112)
		tracker.ext.toms[note.value - 110] = NoteOn;
}

template <>
void InstrumentalTrack<DrumNote_Legacy, false>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position);
