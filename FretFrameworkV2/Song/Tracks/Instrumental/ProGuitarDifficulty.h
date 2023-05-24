#pragma once
#include "DifficultyTrack.h"
#include "Notes/GuitarNote_Pro.h"

template <int numStrings, int numFrets>
class DifficultyTrack<GuitarNote_Pro<numStrings, numFrets>> : public Track
{
public:
	SimpleFlatMap<GuitarNote_Pro<numStrings, numFrets>> m_notes;
	SimpleFlatMap<ArpeggioNote<numFrets>> m_arpeggios;

public:
	virtual void adjustTicks(double multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& note : m_notes)
		{
			note.key = uint64_t(note.key * multiplier);
			*note *= multiplier;
		}

		for (auto& arp : m_arpeggios)
		{
			arp.key = uint64_t(arp.key * multiplier);
			*arp *= multiplier;
		}
	}

	virtual void clear() override
	{
		Track::clear();
		m_notes.clear();
		m_arpeggios.clear();
	}

	[[nodiscard]] virtual bool isOccupied() const override { return !m_notes.isEmpty() || !m_arpeggios.isEmpty() || Track::isOccupied(); }

	void shrink()
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}
};
