#pragma once
#include "InstrumentalTrack.h"
#include "Notes/DrumNote_Legacy.h"

namespace LegacyDrums
{
	template <class DrumType, size_t numPads>
	static void Transfer(InstrumentalTrack<DrumNote_Legacy>& from, InstrumentalTrack<DrumNote<DrumType, numPads>>& to)
	{
		to.m_specialPhrases = std::move(from.m_specialPhrases);
		to.m_events = std::move(from.m_events);
		for (size_t i = 0; i < 5; ++i)
		{
			if (!to.m_difficulties[i].isOccupied() && from.m_difficulties[i].isOccupied())
			{
				to.m_difficulties[i].m_specialPhrases = std::move(from.m_difficulties[i].m_specialPhrases);
				to.m_difficulties[i].m_events = std::move(from.m_difficulties[i].m_events);
				to.m_difficulties[i].m_notes.reserve(from.m_difficulties[i].m_notes.size());
				for (const auto& note : from.m_difficulties[i].m_notes)
					to.m_difficulties[i].m_notes.emplace_back(note.key, note->transformNote<DrumType, numPads>());
				from.m_difficulties[i].m_notes.clear();
			}
		}
	}
}
