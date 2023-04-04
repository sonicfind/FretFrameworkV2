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
	virtual ~Track() {}

	virtual void load(CommonChartParser* parser) = 0;
	virtual void save(CommonChartWriter* writer) const = 0;

	[[nodiscard]] virtual bool hasNotes() const = 0;

	// Returns whether this track contains notes, special phrases, or other events
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

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplacePhrases(uint32_t position)
	{
		return m_specialPhrases[position];
	}

	[[nodiscard]] std::vector<std::u32string>& get_or_emplaceEvents(uint32_t position)
	{
		return m_events[position];
	}

	[[nodiscard]] const std::vector<SpecialPhrase>& getPhrases(uint32_t position) const
	{
		return m_specialPhrases.at(position);
	}

	[[nodiscard]] const std::vector<std::u32string>& getEvents(uint32_t position) const
	{
		return m_events.at(position);
	}

protected:
	[[nodiscard]] std::vector<std::u32string>& get_or_emplace_Events_midi(uint32_t position)
	{
		return m_events.get_or_emplace_back(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplace_SpecialPhrase_midi(uint32_t position)
	{
		return m_specialPhrases.get_or_emplaceNodeFromBack(position);
	}

protected:
	SimpleFlatMap<std::vector<SpecialPhrase>> m_specialPhrases;
	SimpleFlatMap<std::vector<std::u32string>> m_events;
};
