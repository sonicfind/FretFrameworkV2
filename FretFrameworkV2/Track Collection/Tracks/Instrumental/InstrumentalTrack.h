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
		for (size_t i = 0; i < 5; ++i)
			if (!m_difficulties[i].isOccupied())
				m_difficulties[i] = std::move(track.m_difficulties[i]);
		return *this;
	}

	void load_V1(size_t diff, TxtFileReader& reader)
	{
		m_difficulties[diff].load_V1(reader);
	}

	virtual void load(CommonChartParser* parser) override
	{
		while (parser->isStillCurrentTrack())
		{
			if (!parser->isStartOfTrack())
			{
				parse_event(parser);
				parser->nextEvent();
			}
			else
			{
				if (parser->validateDifficultyTrack())
				{
					const size_t diff = parser->getDifficulty();
					if (diff < 5)
					{
						parser->nextEvent();
						m_difficulties[diff].load(parser);
					}
					else // BCH only
						parser->skipTrack();

				}
				else if (parser->validateAnimationTrack())
				{
					parser->nextEvent();
					load_anim(parser);
				}
				else
				{
					parser->processUnknownTrack();
					parser->skipTrack();
				}
				parser->endTrack();
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

	void clear()
	{
		m_specialPhrases.clear();
		m_events.clear();
		for (auto& diff : m_difficulties)
			diff.clear();
	}

	void addNote(size_t diffIndex, uint32_t position, int note, uint32_t sustain = 0)
	{
		m_difficulties[diffIndex].addNote(position, note, sustain);
	}

	void addPhrase(size_t diffIndex, uint32_t position, SpecialPhrase phrase)
	{
		m_difficulties[diffIndex].addPhrase(position, phrase);
	}

	void addEvent(size_t diffIndex, uint32_t position, std::string_view str)
	{
		m_difficulties[diffIndex].addEvent(position, str);
	}

	void modifyNote(size_t diffIndex, uint32_t position, char modifier, int lane = 0)
	{
		m_difficulties[diffIndex].modifyNote(position, modifier, lane);
	}

	void addSharedPhrase(uint32_t position, SpecialPhrase phrase)
	{
		m_specialPhrases[position].push_back(phrase);
	}

	void addSharedEvent(uint32_t position, std::string_view str)
	{
		if (str[0] == '\"')
			str = str.substr(1, str.length() - 2);
		m_events[position].push_back(str);
	}

	void adjustTicks(float multiplier)
	{
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);
	}

private:
	void parse_event(CommonChartParser* parser)
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
	}

	void load_anim(CommonChartParser* parser)
	{
		while (parser->isStillCurrentTrack())
		{
			uint32_t position = parser->parsePosition();
			ChartEvent ev = parser->parseEvent();
			parser->nextEvent();
		}
	}

	void save_events(CommonChartWriter* writer) const
	{
		SimpleFlatMap<WriteNode> nodes(m_specialPhrases.size() + m_events.size());
		for (const auto& note : m_specialPhrases)
			nodes.try_construct_back(note.key)->m_phrases = &note.object;

		for (const auto& events : m_events)
			nodes[events.key].m_events = &events.object;

		for (const auto& node : nodes)
		{
			node->writePhrases(node.key, writer);
			node->writeEvents(node.key, writer);
		}
	}

	void save_anim(CommonChartWriter* parser) const
	{
	}

private:
	T* construct_note_midi(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].construct_note_midi(position);
	}

	void addNote_midi(size_t diff, uint32_t position, int note, uint32_t sustain = 1)
	{
		m_difficulties[diff].addNote_midi(position, note, sustain);
	}

	T& backNote_midiOnly(size_t diff)
	{
		return m_difficulties[diff].backNote_midiOnly();
	}

	void construct_phrase_midi(uint32_t position)
	{
		m_specialPhrases.try_construct_back(position);
	}

	void construct_phrase_midi(size_t diff, uint32_t position)
	{
		m_difficulties[diff].construct_phrase_midi(position);
	}

	void addEvent_midi(uint32_t position, std::string_view str)
	{
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
	}

	void addSpecialPhrase_midi(uint32_t position, SpecialPhrase phrase)
	{
		auto iter = m_specialPhrases.end() - 1;
		while (iter->key > position)
			--iter;

		(*iter)->push_back(phrase);
	}

	void addSpecialPhrase_midi(size_t diff, uint32_t position, SpecialPhrase phrase)
	{
		m_difficulties[diff].addSpecialPhrase_midi(position, phrase);
	}

	T* testBackNote_midiOnly(size_t diff, uint32_t position)
	{
		return m_difficulties[diff].testBackNote_midiOnly(position);
	}

	void modifyBackNote_midiOnly(size_t diff, uint32_t position, char modifier, size_t lane = 0)
	{
		m_difficulties[diff].modifyBackNote_midiOnly(position, modifier, lane);
	}

	void convertSoloesToStarPower_midi()
	{
		for (auto iter = m_specialPhrases.begin(); iter < m_specialPhrases.end();)
		{
			for (auto phraseIter = (*iter)->begin(); phraseIter < (*iter)->end();)
			{
				if (phraseIter->getMidiNote() == 103)
				{
					m_difficulties[3].addPhrase(iter->key, { SpecialPhraseType::StarPower, phraseIter->getDuration() });
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

private:
	DifficultyTrack<T> m_difficulties[5];
};
