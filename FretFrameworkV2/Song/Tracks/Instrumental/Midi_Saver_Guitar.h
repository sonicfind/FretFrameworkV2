#pragma once
#include "Midi_Saver_Instrument.h"
#include "Notes/GuitarNote.h"
#include "Midi_Hold.h"

namespace MidiGuitar
{
	struct ForceHold : public Midi_Details::Hold
	{
		ForceStatus status = ForceStatus::UNFORCED;
	};

	template <size_t numFrets>
	void WriteModifiers(const SimpleFlatMap<GuitarNote<numFrets>>& notes, unsigned char index, MidiFileWriter& writer)
	{
		const unsigned char FORCE_OFFSET = 65 + 12 * index;

		Midi_Details::Hold tap;
		ForceHold forcing;
		Midi_Details::Hold open;
		for (const auto& node : notes)
		{
			if (node->isTapped())
			{
				if (tap.start == UINT64_MAX)
					tap.start = node.key;
				tap.end = node.key + node->getLongestSustain();
			}
			else if (tap.start != UINT64_MAX)
			{
				if (tap.end > node.key)
					tap.end = node.key;

				writer.addSysex(tap.start, index, 4, tap.getLength());
				tap.start = UINT64_MAX;
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

					writer.addMidiNote(forcing.start, FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF), 100, forcing.getLength());

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

			if constexpr (numFrets == 5)
			{
				const Sustained& spec = node->getSpecial();
				if (spec.isActive())
				{
					if (open.start == UINT64_MAX)
						open.start = node.key;
					open.end = node.key + spec.getLength();
				}
				else if (open.start != UINT64_MAX)
				{
					if (open.end > node.key)
						open.end = node.key;

					writer.addSysex(open.start, index, 1, open.getLength());
					open.start = UINT64_MAX;
				}
			}
		}

		if (tap.start != UINT64_MAX)
			writer.addSysex(tap.start, index, 4, tap.getLength());

		if (forcing.status != ForceStatus::UNFORCED)
			writer.addMidiNote(forcing.start, FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF), 100, forcing.getLength());

		if constexpr (numFrets == 5)
			if (open.start != UINT64_MAX)
				writer.addSysex(open.start, index, 1, open.getLength());
	}
}

template <>
void Difficulty_Saver_Midi<GuitarNote<5>>::write_details(const DifficultyTrack<GuitarNote<5>>& track, const unsigned char index, MidiFileWriter& writer)
{
	MidiGuitar::WriteModifiers(track.m_notes, index, writer);
}

template <>
void Difficulty_Saver_Midi<GuitarNote<6>>::write_details(const DifficultyTrack<GuitarNote<6>>& track, const unsigned char index, MidiFileWriter& writer)
{
	MidiGuitar::WriteModifiers(track.m_notes, index, writer);
}
