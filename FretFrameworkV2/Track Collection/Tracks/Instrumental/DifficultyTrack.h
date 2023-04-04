#pragma once
#include "../Track.h"
#include "File Processing/TxtFileReader.h"

template <typename T>
class InstrumentalTrack;

template <typename T>
class DifficultyTrack : private Track
{
	friend class InstrumentalTrack<T>;

public:
	template <typename U>
	DifficultyTrack& operator=(DifficultyTrack<U>&& diff)
	{
		if constexpr (std::is_same<T, U>::value)
		{
			m_events = std::move(diff.m_events);
			m_specialPhrases = std::move(diff.m_specialPhrases);
			m_notes = std::move(diff.m_notes);
		}
		return *this;
	}

	[[nodiscard]] T& get_or_emplaceNote(uint32_t position)
	{
		return m_notes[position];
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplacePhrases(uint32_t position)
	{
		return m_specialPhrases[position];
	}

	[[nodiscard]] std::vector<std::u32string>& get_or_emplaceEvents(uint32_t position)
	{
		return m_events[position];
	}

	[[nodiscard]] T& getNote(uint32_t position)
	{
		return m_notes.at(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& getPhrases(uint32_t position)
	{
		return m_specialPhrases.at(position);
	}

	[[nodiscard]] std::vector<std::u32string>& getEvents(uint32_t position)
	{
		return m_events.at(position);
	}

	[[nodiscard]] uint32_t getNumActive(uint32_t position)
	{
		return m_notes.at(position).getNumActive();
	}

	virtual void clear() override
	{
		m_notes.clear();
		m_events.clear();
		m_specialPhrases.clear();
	}

	// Returns whether this difficulty contains notes
	// ONLY checks for notes
	[[nodiscard]] virtual bool hasNotes() const override { return m_notes.size(); }

	// Returns whether this difficulty contains notes, effects, soloes, or other events
	[[nodiscard]] virtual bool isOccupied() const override { return !m_notes.isEmpty() || !m_events.isEmpty() || !m_specialPhrases.isEmpty(); }

private:
	void load_V1(TxtFileReader& reader)
	{
		if (isOccupied())
			throw std::runtime_error("Track defined multiple times in file");

		uint32_t solo = 0;

		m_notes.reserve(5000);
		reader.nextEvent();
		while (reader.isStillCurrentTrack())
		{
			const uint32_t position = reader.parsePosition();
			switch (reader.parseEvent())
			{
			case ChartEvent::NOTE:
			{
				auto& note = m_notes.get_or_emplace_back(position);
				auto values = reader.extractColorAndSustain_V1();
				if (!note.set_V1(values.first, values.second))
					throw std::runtime_error("Note color is invalid");
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = reader.extractSpecialPhrase();
				switch (phrase.getType())
				{
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::StarPowerActivation:
				case SpecialPhraseType::Tremolo:
				case SpecialPhraseType::Trill:
					m_specialPhrases.get_or_emplace_back(position).push_back(phrase);
				}
				break;
			}
			case ChartEvent::EVENT:
			{
				std::string_view str = reader.extractText();
				if (str.starts_with("soloend"))
					get_or_emplacePhrases(position).push_back({ SpecialPhraseType::Solo, position - solo });
				else if (str.starts_with("solo"))
					solo = position;
				else
					m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
				break;
			}
			}
			reader.nextEvent();
		}

		shrink();
	}

	virtual void load(CommonChartParser* parser) override
	{
		if (isOccupied())
			throw std::runtime_error("Difficulty defined multiple times in file");

		m_notes.reserve(5000);
		parser->nextEvent();
		while (parser->isStillCurrentTrack())
		{
			const uint32_t position = parser->parsePosition();
			switch (parser->parseEvent())
			{
			case ChartEvent::NOTE:
			{
				auto& note = m_notes.get_or_emplace_back(position);
				const auto color = parser->extractSingleNote();
				if (!note.set(color.first, color.second))
					throw std::runtime_error("Note color is invalid");

				const auto& modifiers = parser->extractSingleNoteMods();
				for (const auto mod : modifiers)
					note.modify(mod, color.first);
				break;
			}
			case ChartEvent::MULTI_NOTE:
			{
				auto& note = m_notes.get_or_emplace_back(position);
				const auto colors = parser->extractMultiNote();
				for (const auto& color : colors)
					if (!note.set(color.first, color.second))
						throw std::runtime_error("Note color is invalid");
				break;
			}
			case ChartEvent::MODIFIER:
			{
				auto& note = m_notes.back(position);

				const auto& modifiers = parser->extractMultiNoteMods();
				for (const auto& node : modifiers)
					note.modify(node.first, node.second);
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = parser->extractSpecialPhrase();
				switch (phrase.getType())
				{
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::Solo:
				case SpecialPhraseType::StarPowerActivation:
				case SpecialPhraseType::Tremolo:
				case SpecialPhraseType::Trill:
					m_specialPhrases.get_or_emplace_back(position).push_back(phrase);
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
		shrink();
	}

	enum class PointerType
	{
		Note,
		Phrases,
		Events,
	};

	struct DiffWriteNode : public WriteNode
	{
		const T* m_note = nullptr;

		void writeNote(const uint32_t position, CommonChartWriter* writer) const
		{
			if (m_note == nullptr)
				return;

			std::vector<std::pair<size_t, uint32_t>> colors = m_note->getActiveColors();
			if (colors.size() == 1)
			{
				writer->startEvent(position, ChartEvent::NOTE);
				writer->writeSingleNote(colors[0]);
				writer->writeSingleNoteMods(m_note->getActiveModifiers(colors[0].first));
				writer->finishEvent();
			}
			else if (colors.size() > 1)
			{
				writer->startEvent(position, ChartEvent::MULTI_NOTE);
				writer->writeMultiNote(colors);
				writer->finishEvent();

				const std::vector<std::pair<char, size_t>> modifiers = m_note->getActiveModifiers();
				if (!modifiers.empty())
				{
					writer->startEvent(position, ChartEvent::MODIFIER);
					writer->writeMultiNoteMods(modifiers);
					writer->finishEvent();
				}
			}
		}
	};

	virtual void save(CommonChartWriter* writer) const override
	{
		SimpleFlatMap<DiffWriteNode> nodes(m_notes.size() + m_specialPhrases.size() + m_events.size());
		for (const auto& phrases : m_specialPhrases)
			nodes.emplace_back(phrases.key).m_phrases = &phrases.object;

		for (const auto& note : m_notes)
			nodes[note.key].m_note = &note.object;

		for (const auto& events : m_events)
			nodes[events.key].m_events = &events.object;

		for (const auto& node : nodes)
		{
			node->writePhrases(node.key, writer);
			node->writeNote(node.key, writer);
			node->writeEvents(node.key, writer);
		}
	}

	virtual void adjustTicks(float multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& note : m_notes)
		{
			note.key = uint32_t(note.key * multiplier);
			*note *= multiplier;
		}
	}

	void shrink()
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}

	[[nodiscard]] T* construct_note_midi(uint32_t position)
	{
		if (m_notes.capacity() == 0)
			m_notes.reserve(5000);
		return m_notes.try_emplace_back(position);
	}

	[[nodiscard]] T& get_or_construct_note_midi(uint32_t position)
	{
		if (m_notes.capacity() == 0)
			m_notes.reserve(5000);
		return m_notes.get_or_emplace_back(position);
	}

	[[nodiscard]] T& getNote_midi(uint32_t position)
	{
		return m_notes.getNodeFromBack(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplace_SpecialPhrase_midi(uint32_t position)
	{
		return m_specialPhrases.get_or_emplaceNodeFromBack(position);
	}

	[[nodiscard]] T& backNote_midiOnly()
	{
		return m_notes.back();
	}

	[[nodiscard]] T* testBackNote_midiOnly(uint32_t position)
	{
		return m_notes.try_back(position);
	}

private:
	SimpleFlatMap<T> m_notes;
};

