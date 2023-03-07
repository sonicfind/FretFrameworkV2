#pragma once
#include "File Processing/CommonChartParser.h"
#include "File Processing/CommonChartWriter.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Unicode/UnicodeString.h"
#include <fstream>

class Track
{
protected:
	struct WriteNode
	{
		const std::vector<SpecialPhrase>* m_phrases = nullptr;
		const std::vector<std::u32string>* m_events = nullptr;

		void writePhrases(const uint32_t position, CommonChartWriter* writer) const
		{
			if (m_phrases == nullptr)
				return;

			for (const auto& phrase : *m_phrases)
			{
				writer->startEvent(position, ChartEvent::SPECIAL);
				writer->writeSpecialPhrase(phrase);
				writer->finishEvent();
			}
		}

		void writeEvents(const uint32_t position, CommonChartWriter* writer) const
		{
			if (m_events == nullptr)
				return;

			for (const auto& str : *m_events)
			{
				writer->startEvent(position, ChartEvent::EVENT);
				writer->writeText(UnicodeString::U32ToStr(str));
				writer->finishEvent();
			}
		}
	};

public:
	virtual void load(CommonChartParser* parser) = 0;
	virtual void save(CommonChartWriter* writer) const = 0;

	// Returns whether any difficulty in this track contains notes
	// ONLY checks for notes
	[[nodiscard]] virtual bool hasNotes() const = 0;
	// Returns whether this track contains notes, special phrases, or other events
	[[nodiscard]] virtual bool isOccupied() const = 0;
	virtual void clear() = 0;
	virtual void adjustTicks(float multiplier) = 0;
	virtual ~Track() {}

protected:
	SimpleFlatMap<std::vector<SpecialPhrase>> m_specialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_events;
};
