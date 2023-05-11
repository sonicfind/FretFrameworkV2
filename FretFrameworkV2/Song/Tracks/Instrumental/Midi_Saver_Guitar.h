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
	void WriteModifiers(const SimpleFlatMap<GuitarNote<numFrets>>& notes, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& noteList, const unsigned char diff)
	{
		const unsigned char FORCE_OFFSET = 65 + 12 * diff;

		Midi_Details::Hold tap;
		ForceHold forcing;
		Midi_Details::Hold open;
		size_t forceIndex = 0;
		for (const auto& note : notes)
		{
			if (note->isTapped())
			{
				if (tap.start == UINT64_MAX)
					tap.start = note.key;
				tap.end = note.key + note->getLongestSustain();
			}
			else if (tap.start != UINT64_MAX)
			{
				if (tap.end > note.key)
					tap.end = note.key;

				sysexs[tap.start].push_back({ { diff, 4 }, tap.end });
				tap.start = UINT64_MAX;
			}

			if constexpr (numFrets == 5)
			{
				const Sustained& spec = note->getSpecial();
				if (spec.isActive())
				{
					if (open.start == UINT64_MAX)
						open.start = note.key;
					open.end = note.key + spec.getLength();
				}
				else if (open.start != UINT64_MAX)
				{
					if (open.end > note.key)
						open.end = note.key;

					sysexs[open.start].push_back({ { diff, 1 }, open.end });
					open.start = UINT64_MAX;
				}
			}

			ForceStatus forceStatus = note->getForcing();
			if (forcing.status != forceStatus)
			{
				if (forcing.status == ForceStatus::UNFORCED)
				{
					forcing.status = forceStatus;
					forcing.start = note.key;
					forcing.end = note.key + note->getLongestSustain();
					forceIndex = noteList.find_or_emplace(forceIndex, forcing.start);
				}
				else
				{
					if (forcing.end > note.key)
						forcing.end = note.key;

					noteList.at_index(forceIndex)->push_back({ { 0, (unsigned char)(FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF)), 100 }, forcing.end });

					forcing.status = forceStatus;
					if (forceStatus != ForceStatus::UNFORCED)
					{
						forcing.start = note.key;
						forcing.end = note.key + note->getLongestSustain();
					}
				}
			}
			else if (forcing.status != ForceStatus::UNFORCED)
				forcing.end = note.key + note->getLongestSustain();
		}

		if (tap.start != UINT64_MAX)
			sysexs[tap.start].push_back({ { diff, 4 }, tap.end });

		if constexpr (numFrets == 5)
			if (open.start != UINT64_MAX)
				sysexs[open.start].push_back({ { diff, 1 }, open.end });

		if (forcing.status != ForceStatus::UNFORCED)
			noteList.at_index(forceIndex)->push_back({ { 0, (unsigned char)(FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF)), 100 }, forcing.end });
	}
}

template <>
bool Difficulty_Saver_Midi::Get_Details<GuitarNote<5>>(const DifficultyTrack<GuitarNote<5>>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff)
{
	MidiGuitar::WriteModifiers(track.m_notes, sysexs, notes, diff);
	return true;
}

template <>
bool Difficulty_Saver_Midi::Get_Details<GuitarNote<6>>(const DifficultyTrack<GuitarNote<6>>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff)
{
	MidiGuitar::WriteModifiers(track.m_notes, sysexs, notes, diff);
	return true;
}
