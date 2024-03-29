#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Types/UnicodeString.h"
#include "Notes/SpecialPhrase.h"

class Track
{
public:
	SimpleFlatMap<std::vector<SpecialPhrase>> m_specialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_events;

public:
	virtual ~Track() {}
	[[nodiscard]] virtual bool isOccupied() const
	{
		return !m_specialPhrases.isEmpty() || !m_events.isEmpty();
	}

	virtual void clear()
	{
		m_specialPhrases.clear();
		m_events.clear();
	}

	virtual void adjustTicks(double multiplier)
	{
		for (auto& vec : m_specialPhrases)
		{
			vec.key = uint64_t(vec.key * multiplier);
			for (auto& special : *vec)
				special *= multiplier;
		}

		for (auto& ev : m_events)
			ev.key = uint64_t(ev.key * multiplier);
	}
};
