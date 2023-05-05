#pragma once
#include "InstrumentalTrack.h"
#include "Notes/GuitarNote.h"
#include "Midi_Hold.h"

template <>
struct InstrumentalTrack<GuitarNote<5>, true>::Midi_Tracker_Diff
{
	bool sliderNotes = false;
	bool hopoOn = false;
	bool hopoOff = false;
	std::pair<SpecialPhraseType, uint32_t> starPower = { SpecialPhraseType::StarPower_Diff, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> faceOff[2] = { { SpecialPhraseType::FaceOff_Player1, UINT32_MAX } , { SpecialPhraseType::FaceOff_Player2, UINT32_MAX } };
	uint32_t notes[6] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
struct InstrumentalTrack<GuitarNote<6>, true>::Midi_Tracker_Diff
{
	bool sliderNotes = false;
	bool hopoOn = false;
	bool hopoOff = false;
	uint32_t notes[7] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalTrack<GuitarNote<5>, true>::s_noteRange{ 59, 107 };

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalTrack<GuitarNote<6>, true>::s_noteRange{ 58, 103 };

template <>
constexpr int InstrumentalTrack<GuitarNote<5>, true>::Midi_Tracker::s_defaultLanes[48] =
{
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};

template <>
constexpr int InstrumentalTrack<GuitarNote<6>, true>::Midi_Tracker::s_defaultLanes[48] =
{
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
	0, 4, 5, 6, 1, 2, 3, 7, 8, 9, 10, 11,
};

template <>
template <bool NoteOn>
void InstrumentalTrack<GuitarNote<5>, true>::parseLaneColor(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	auto replaceSoloesWithStarPower = [&]
	{
		for (size_t i = 0; i < 4; ++i)
			tracker.laneValues[12 * i + 8] = 12;

		for (auto iter = m_specialPhrases.begin(); iter < m_specialPhrases.end();)
		{
			for (auto phraseIter = (*iter)->begin(); phraseIter < (*iter)->end();)
			{
				if (phraseIter->getMidiNote() == 103)
				{
					m_difficulties[3].m_specialPhrases[iter->key].push_back({ SpecialPhraseType::StarPower, phraseIter->getDuration() });
					phraseIter = (*iter)->erase(phraseIter);
				}
				else
					++phraseIter;
			}

			if ((*iter)->empty())
				iter = m_specialPhrases.erase(iter);
			else
				++iter;
		}
	};

	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];

	if (lane == -1)
		return;

	const int diff = s_diffValues[noteValue];
	if (lane < 6)
	{
		if constexpr (NoteOn)
		{
			tracker.difficulties[diff].notes[lane] = position;

			if (auto note = m_difficulties[diff].construct_note_midi(position))
			{
				if (tracker.difficulties[diff].sliderNotes)
					note->setTap(true);

				if (tracker.difficulties[diff].hopoOn)
					note->setForcing(ForceStatus::HOPO_ON);
				else if (tracker.difficulties[diff].hopoOff)
					note->setForcing(ForceStatus::HOPO_OFF);
			}
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
	// HopoON marker
	else if (lane == 6)
	{
		tracker.difficulties[diff].hopoOn = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_difficulties[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 7)
	{
		tracker.difficulties[diff].hopoOff = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_difficulties[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
	else if (lane == 8)
	{
		if (diff == 3)
		{
			addSpecialPhrase<NoteOn>(tracker.solo, position);
			return;
		}

		replaceSoloesWithStarPower();
		addSpecialPhrase<NoteOn>(tracker.difficulties[diff].starPower, diff, position);
	}
	else if (lane == 9)
		tracker.difficulties[diff].sliderNotes = NoteOn;
	else if (lane == 10)
		addSpecialPhrase<NoteOn>(tracker.difficulties[diff].faceOff[0], diff, position);
	else if (lane == 11)
		addSpecialPhrase<NoteOn>(tracker.difficulties[diff].faceOff[1], diff, position);
	else if (lane == 12)
		addSpecialPhrase<NoteOn>(tracker.difficulties[diff].starPower, diff, position);
}

template <>
template <bool NoteOn>
void InstrumentalTrack<GuitarNote<6>, true>::parseLaneColor(Midi_Tracker& tracker, MidiNote note, uint32_t position)
{
	const int noteValue = note.value - s_noteRange.first;
	const int lane = tracker.laneValues[noteValue];
	const int diff = s_diffValues[noteValue];

	if (lane < 7)
	{
		if constexpr (NoteOn)
		{
			tracker.difficulties[diff].notes[lane] = position;

			if (auto note = m_difficulties[diff].construct_note_midi(position))
			{
				if (tracker.difficulties[diff].sliderNotes)
					note->setTap(true);

				if (tracker.difficulties[diff].hopoOn)
					note->setForcing(ForceStatus::HOPO_ON);
				else if (tracker.difficulties[diff].hopoOff)
					note->setForcing(ForceStatus::HOPO_OFF);
			}
		}
		else
		{
			uint32_t& colorPosition = tracker.difficulties[diff].notes[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_difficulties[diff].m_notes.getNodeFromBack(colorPosition).set(lane, position - colorPosition);
				colorPosition = UINT32_MAX;
			}
		}
	}
	// HopoON marker
	else if (lane == 7)
	{
		tracker.difficulties[diff].hopoOn = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_difficulties[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_ON);
		}
	}
	// HopoOff marker
	else if (lane == 8)
	{
		tracker.difficulties[diff].hopoOff = NoteOn;
		if constexpr (NoteOn)
		{
			if (auto note = m_difficulties[diff].m_notes.try_back(position))
				note->setForcing(ForceStatus::HOPO_OFF);
		}
	}
	else if (lane == 10)
		tracker.difficulties[diff].sliderNotes = NoteOn;
}

template <>
void InstrumentalTrack<GuitarNote<5>, true>::parseSysEx(Midi_Tracker& tracker, std::string_view str, uint32_t position);

template <>
void InstrumentalTrack<GuitarNote<6>, true>::parseSysEx(Midi_Tracker& tracker, std::string_view str, uint32_t position);

template <>
void InstrumentalTrack<GuitarNote<5>, true>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position);

template <>
void InstrumentalTrack<GuitarNote<5>, true>::writeMidiToggleEvent(MidiFileWriter& writer) const;

namespace MidiDrums
{
	struct ForceHold : public Midi_Details::Hold
	{
		ForceStatus status = ForceStatus::UNFORCED;
	};

	template <unsigned char INDEX, size_t numFrets>
	void WriteModifiers(MidiFileWriter& writer, const SimpleFlatMap<GuitarNote<numFrets>>& notes)
	{
		static constexpr unsigned char FORCE_OFFSET = 65 + 12 * INDEX;
		static constexpr unsigned char TAP_VALUE = 68 + 12 * INDEX;

		Midi_Details::Hold tap;
		ForceHold forcing;
		for (const auto& node : notes)
		{
			if (node->isTapped())
			{
				if (tap.start == UINT32_MAX)
					tap.start = node.key;
				tap.end = node.key + node->getLongestSustain();
			}
			else if (tap.start != UINT32_MAX)
			{
				if (tap.end > node.key)
					tap.end = node.key;

				writer.addMidiNote(tap.start, TAP_VALUE, 100, tap.end - tap.start);
				tap.start = UINT32_MAX;
			}

			ForceStatus forceStatus = node->getForcing();
			if (forcing.status != forceStatus)
			{
				if (forcing.status == ForceStatus::UNFORCED)
				{
					forcing.status = forceStatus;
					forcing.start = node.key;
					forcing.end = node.key + node->getLongestSustain();
				}
				else
				{
					if (forcing.end > node.key)
						forcing.end = node.key;

					writer.addMidiNote(forcing.start, FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF), 100, forcing.end - forcing.start);

					forcing.status = forceStatus;
					if (forceStatus != ForceStatus::UNFORCED)
					{
						forcing.start = node.key;
						forcing.end = node.key + node->getLongestSustain();
					}
				}
			}
			else if (forcing.status != ForceStatus::UNFORCED)
				forcing.end = node.key + node->getLongestSustain();
		}

		if (tap.start != UINT32_MAX)
			writer.addMidiNote(tap.start, TAP_VALUE, 100, tap.end - tap.start);

		if (forcing.status != ForceStatus::UNFORCED)
			writer.addMidiNote(forcing.start, FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF), 100, forcing.end - forcing.start);
	}
}

template <>
template <unsigned char INDEX>
void DifficultyTrack<GuitarNote<5>, false>::write_details(MidiFileWriter& writer, bool& doDetails) const
{
	MidiDrums::WriteModifiers<INDEX>(writer, m_notes);
}

template <>
template <unsigned char INDEX>
void DifficultyTrack<GuitarNote<6>, false>::write_details(MidiFileWriter& writer, bool& doDetails) const
{
	MidiDrums::WriteModifiers<INDEX>(writer, m_notes);
}
