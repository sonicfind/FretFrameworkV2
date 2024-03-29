#pragma once
#include "../Track.h"

template <typename T>
class DifficultyTrack : public Track
{
public:
	SimpleFlatMap<T> m_notes;

public:
	virtual void adjustTicks(double multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& note : m_notes)
		{
			note.key = uint64_t(note.key * multiplier);
			*note *= multiplier;
		}
	}

	virtual void clear() override
	{
		Track::clear();
		m_notes.clear();
	}

	[[nodiscard]] virtual bool isOccupied() const override { return !m_notes.isEmpty() || Track::isOccupied(); }

	void shrink()
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}
};
