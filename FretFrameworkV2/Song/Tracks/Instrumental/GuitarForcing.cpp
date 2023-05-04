#include "GuitarForcing.h"

template <size_t numFrets>
void FixForcing(SimpleFlatMap<GuitarNote<numFrets>>& notes, const uint32_t forceThreshold)
{
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

template<>
void InstrumentalTrack<GuitarNote<5>, true>::load_V1(size_t diff, ChtFileReader& reader)
{
	m_difficulties[diff].load_V1(reader);
	FixForcing(m_difficulties[diff].m_notes, reader.getHopoThreshold());
}

template<>
void InstrumentalTrack<GuitarNote<6>, true>::load_V1(size_t diff, ChtFileReader& reader)
{
	m_difficulties[diff].load_V1(reader);
	FixForcing(m_difficulties[diff].m_notes, reader.getHopoThreshold());
}
