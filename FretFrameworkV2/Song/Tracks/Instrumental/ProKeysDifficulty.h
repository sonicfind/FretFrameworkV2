#pragma once
#include "DifficultyTrack.h"
#include "Notes/Keys_Pro.h"

template<>
class DifficultyTrack<Keys_Pro> : public Track
{
public:
	SimpleFlatMap<Keys_Pro> m_notes;
	SimpleFlatMap<ProKey_Ranges> m_ranges;

public:
	virtual void adjustTicks(double multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& note : m_notes)
		{
			note.key = uint64_t(note.key * multiplier);
			*note *= multiplier;
		}

		for (auto& range : m_ranges)
			range.key = uint64_t(range.key * multiplier);
	}

	virtual void clear() override
	{
		Track::clear();
		m_notes.clear();
		m_ranges.clear();
	}

	[[nodiscard]] virtual bool isOccupied() const override { return !m_notes.isEmpty() || !m_ranges.isEmpty() || Track::isOccupied(); }

	void shrink()
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}
};
