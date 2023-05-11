#pragma once
#include "InstrumentalTrack.h"
#include "Notes/GuitarNote.h"

namespace ForcingFix
{
	template <size_t numFrets>
	void Fix(InstrumentalTrack<GuitarNote<numFrets>>& track, const uint64_t forceThreshold)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			auto& notes = track[i].m_notes;
			for (size_t i = 0; i < notes.size(); ++i)
			{
				auto& note = notes.at_index(i);
				if (note->getForcing() == ForceStatus::FORCED)
				{
					if (note->isChorded() || i == 0 || note->hasSameFretting(*notes.at_index(i - 1)) || note.key > notes.at_index(i - 1).key + forceThreshold)
						note->setForcing(ForceStatus::HOPO_ON);
					else
						note->setForcing(ForceStatus::HOPO_OFF);
				}
			}
		}
	}
}
