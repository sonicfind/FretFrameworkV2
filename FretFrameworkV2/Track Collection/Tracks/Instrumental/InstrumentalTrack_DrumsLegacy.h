#pragma once
#include "DrumTrack_midi.h"
#include "Notes/DrumNote_Legacy.h"

class Legacy_DrumTrack : public InstrumentalTrack<DrumNote_Legacy>
{
private:
	using InstrumentalTrack<DrumNote_Legacy>::load;

public:
	Legacy_DrumTrack() = default;
	Legacy_DrumTrack(Legacy_DrumTrack&&) = default;
	Legacy_DrumTrack& operator=(Legacy_DrumTrack&&) = default;

	Legacy_DrumTrack(const InstrumentalTrack&) = delete;
	Legacy_DrumTrack& operator=(const InstrumentalTrack&) = delete;

	Legacy_DrumTrack(MidiFileReader& reader)
	{
		InstrumentalTrack<DrumNote_Legacy>::load(reader);
		for (size_t i = 0; i < 4; ++i)
		{
			m_type = evaluateDrumType(i);
			if (m_type != DrumType_Enum::LEGACY)
				break;
		}
	}

	void load_V1(size_t diff, TxtFileReader& reader)
	{
		InstrumentalTrack<DrumNote_Legacy>::load_V1(diff, reader);
		m_type = evaluateDrumType(diff);
	}

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
	DrumType_Enum evaluateDrumType(size_t index)
	{
		for (const auto& note : m_difficulties[index].m_notes)
		{
			DrumType_Enum type = note->evaluateDrumType();
			if (type != DrumType_Enum::LEGACY)
				return type;
		}
		return DrumType_Enum::LEGACY;
	}

	DrumType_Enum m_type = DrumType_Enum::LEGACY;
};

template <>
struct Midi_Tracker_Diff<DrumNote_Legacy> : public Midi_Tracker_Diff<DrumNote<5, true>> {};

template <>
struct Midi_Tracker_Extensions<DrumNote_Legacy> : public Midi_Tracker_Extensions<DrumNote<5, true>> {};

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalTrack<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
void InstrumentalTrack<DrumNote_Legacy>::parseLaneColor(Midi_Tracker<DrumNote_Legacy>& tracker, MidiNote note, uint32_t position)
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
void InstrumentalTrack<DrumNote_Legacy>::toggleExtraValues(Midi_Tracker<DrumNote_Legacy>& tracker, MidiNote note, uint32_t position)
{
	if (110 <= note.value && note.value <= 112)
		tracker.ext.toms[note.value - 110] = NoteOn;
}

template <>
void InstrumentalTrack<DrumNote_Legacy>::parseText(Midi_Tracker<DrumNote_Legacy>& tracker, std::string_view str, uint32_t position);
