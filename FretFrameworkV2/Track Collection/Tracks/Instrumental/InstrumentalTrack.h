#pragma once
#include <iostream>
#include "DifficultyTrack.h"

template <class T>
class InstrumentTrackMidiParser;

template <class T>
class InstrumentalTrack : public Track
{
	friend class InstrumentTrackMidiParser<T>;
public:
	InstrumentalTrack() = default;
	InstrumentalTrack(InstrumentalTrack&&) = default;
	InstrumentalTrack(const InstrumentalTrack&) = delete;
	InstrumentalTrack& operator=(const InstrumentalTrack&) = delete;

	template <typename U>
	InstrumentalTrack& operator=(InstrumentalTrack<U>&& track)
	{
		if constexpr (std::is_same<T, U>::value)
		{
			m_events = std::move(track.m_events);
			m_specialPhrases = std::move(track.m_specialPhrases);
			for (size_t i = 0; i < 5; ++i)
				if (!m_difficulties[i].isOccupied())
					m_difficulties[i] = std::move(track.m_difficulties[i]);
		}
		return *this;
	}

	void load_V1(size_t diff, TxtFileReader& reader)
	{
		m_difficulties[diff].load_V1(reader);
	}

	virtual void load(CommonChartParser* parser) override
	{
		parser->nextEvent();
		while (parser->isStillCurrentTrack())
		{
			if (!parser->isStartOfTrack())
				parse_event(parser);
			else
			{
				if (parser->validateDifficultyTrack())
				{
					const size_t diff = parser->getDifficulty();
					if (diff < 5)
						m_difficulties[diff].load(parser);
					else // BCH only
						parser->skipUnknownTrack();

				}
				else if (parser->validateAnimationTrack())
					load_anim(parser);
				else
					parser->skipUnknownTrack();
			}
		}
	}

	virtual void save(CommonChartWriter* writer) const override
	{
		save_events(writer);
		save_anim(writer);
		for (size_t i = 0; i < 5; ++i)
			if (m_difficulties[i].isOccupied())
			{
				writer->writeDifficultyTrack(i);
				m_difficulties[i].save(writer);
				writer->finishTrack();
			}
	}

	void save_midi(const char* const name, std::fstream& outFile) const;

	// Returns whether any difficulty in this track contains notes
	// ONLY checks for notes
	[[nodiscard]] virtual bool hasNotes() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.hasNotes())
				return true;
		return false;
	}

	// Returns whether any difficulty in this track contains notes, effects, soloes, or other events
	[[nodiscard]] virtual bool isOccupied() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.isOccupied())
				return true;
		return false;
	}

	virtual void clear()
	{
		m_specialPhrases.clear();
		m_events.clear();
		for (auto& diff : m_difficulties)
			diff.clear();
	}

	[[nodiscard]] T& get_or_emplaceNote(size_t diffIndex, uint32_t position)
	{
		return m_difficulties[diffIndex].get_or_emplaceNote(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplacePhrases(size_t diffIndex, uint32_t position)
	{
		return m_difficulties[diffIndex].get_or_emplacePhrases(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplacePhrases(uint32_t position)
	{
		return m_specialPhrases[position];
	}

	[[nodiscard]] std::vector<std::u32string>& get_or_emplaceEvents(size_t diffIndex, uint32_t position)
	{
		return m_difficulties[diffIndex].get_or_emplaceEvents(position);
	}

	[[nodiscard]] std::vector<std::u32string>& get_or_emplaceEvents(uint32_t position)
	{
		return m_events[position];
	}

	[[nodiscard]] const T& getNote(size_t diffIndex, uint32_t position) const
	{
		return m_difficulties[diffIndex].getNote(position);
	}

	[[nodiscard]] const std::vector<SpecialPhrase>& getPhrases(size_t diffIndex, uint32_t position) const
	{
		return m_difficulties[diffIndex].getPhrases(position);
	}

	[[nodiscard]] const std::vector<SpecialPhrase>& getPhrases(uint32_t position) const
	{
		return m_specialPhrases.at(position);
	}

	[[nodiscard]] const std::vector<std::u32string>& getEvents(size_t diffIndex, uint32_t position) const
	{
		return m_difficulties[diffIndex].getEvents(position);
	}

	[[nodiscard]] const std::vector<std::u32string>& getEvents(uint32_t position) const
	{
		return m_events.at(position);
	}

	virtual void adjustTicks(float multiplier)
	{
		Track::adjustTicks(multiplier);
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);
	}

protected:
	virtual void parse_event(CommonChartParser* parser)
	{
		const uint32_t position = parser->parsePosition();
		switch (parser->parseEvent())
		{
		case ChartEvent::SPECIAL:
		{
			auto& phrases = m_specialPhrases.get_or_emplace_back(position);
			auto phrase = parser->extractSpecialPhrase();
			switch (phrase.getType())
			{
			case SpecialPhraseType::StarPower:
			case SpecialPhraseType::Solo:
			case SpecialPhraseType::StarPowerActivation:
			case SpecialPhraseType::Tremolo:
			case SpecialPhraseType::Trill:
				phrases.push_back(phrase);
			}
			break;
		}
		case ChartEvent::EVENT:
		{
			auto& events = m_events.get_or_emplace_back(position);
			events.push_back(UnicodeString::strToU32(parser->extractText()));
			break;
		}
		}
		parser->nextEvent();
	}

	virtual void load_anim(CommonChartParser* parser)
	{
		parser->nextEvent();
		while (parser->isStillCurrentTrack())
		{
			uint32_t position = parser->parsePosition();
			ChartEvent ev = parser->parseEvent();
			parser->nextEvent();
		}
	}

	virtual void save_events(CommonChartWriter* writer) const
	{
		SimpleFlatMap<WriteNode> nodes(m_specialPhrases.size() + m_events.size());
		for (const auto& note : m_specialPhrases)
			nodes.emplace_back(note.key).m_phrases = &note.object;

		for (const auto& events : m_events)
			nodes[events.key].m_events = &events.object;

		for (const auto& node : nodes)
		{
			node->writePhrases(node.key, writer);
			node->writeEvents(node.key, writer);
		}
	}

	virtual void save_anim(CommonChartWriter* parser) const
	{
	}

private:
	T* construct_note_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].construct_note_midi(position);
	}

	[[nodiscard]] T& get_or_construct_note_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].get_or_construct_note_midi(position);
	}

	[[nodiscard]] T& backNote_midiOnly(size_t diff)
	{
		return m_difficulties[diff].backNote_midiOnly();
	}

	[[nodiscard]] T* testBackNote_midiOnly(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].testBackNote_midiOnly(position);
	}

	[[nodiscard]] T& getNote_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].getNote_midi(position);
	}

	[[nodiscard]] std::vector<std::u32string>& get_or_emplace_Events_midi(uint32_t position)
	{
		return m_events.get_or_emplace_back(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplace_SpecialPhrase_midi(uint32_t position)
	{
		return m_specialPhrases.get_or_emplaceNodeFromBack(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplace_SpecialPhrase_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].get_or_emplace_SpecialPhrase_midi(position);
	}

	void convertSoloesToStarPower_midi()
	{
		for (auto iter = m_specialPhrases.begin(); iter < m_specialPhrases.end();)
		{
			for (auto phraseIter = (*iter)->begin(); phraseIter < (*iter)->end();)
			{
				if (phraseIter->getMidiNote() == 103)
				{
					m_difficulties[3].get_or_emplacePhrases(iter->key).push_back({ SpecialPhraseType::StarPower, phraseIter->getDuration() });
					phraseIter = (*iter)->erase(phraseIter);
				}
				else
					++phraseIter;
			}

			if ((*iter)->empty())
				iter = m_specialPhrases.erase(iter);
			else
				++iter;
		}
	}

	void shrink_midi()
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

protected:
	DifficultyTrack<T> m_difficulties[5];
};
