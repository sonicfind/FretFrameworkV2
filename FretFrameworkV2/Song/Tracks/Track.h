#pragma once
#include "Serialization/CommonChartParser.h"
#include "Serialization/CommonChartWriter.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Types/UnicodeString.h"
#include <fstream>

class Track
{
public:
	SimpleFlatMap<std::vector<SpecialPhrase>> m_specialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_events;

public:
	virtual ~Track() {}
	[[nodiscard]] virtual bool hasNotes() const { return false; };
	[[nodiscard]] virtual bool isOccupied() const
	{
		return !m_specialPhrases.isEmpty() || !m_events.isEmpty();
	}

	virtual void clear()
	{
		m_specialPhrases.clear();
		m_events.clear();
	}

	virtual void adjustTicks(float multiplier)
	{
		for (auto& vec : m_specialPhrases)
		{
			vec.key = uint32_t(vec.key * multiplier);
			for (auto& special : *vec)
				special *= multiplier;
		}

		for (auto& ev : m_events)
			ev.key = uint32_t(ev.key * multiplier);
	}

protected:
	virtual void shrink() = 0;
};

class BCH_CHT_Extensions
{
protected:
	struct WriteNode
	{
		const std::vector<SpecialPhrase>* m_phrases = nullptr;
		const std::vector<std::u32string>* m_events = nullptr;

		void writePhrases(const uint32_t position, CommonChartWriter& writer) const
		{
			if (m_phrases == nullptr)
				return;

			for (const auto& phrase : *m_phrases)
			{
				writer.startEvent(position, ChartEvent::SPECIAL);
				writer.writeSpecialPhrase(phrase);
				writer.finishEvent();
			}
		}

		void writeEvents(const uint32_t position, CommonChartWriter& writer) const
		{
			if (m_events == nullptr)
				return;

			for (const auto& str : *m_events)
			{
				writer.startEvent(position, ChartEvent::EVENT);
				writer.writeText(UnicodeString::U32ToStr(str));
				writer.finishEvent();
			}
		}
	};
public:
	virtual void load(CommonChartParser& parser) = 0;
	virtual void save(CommonChartWriter& writer) const = 0;
};
