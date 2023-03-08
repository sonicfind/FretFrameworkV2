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
	DifficultyTrack& operator=(DifficultyTrack<U>&& diff) { return *this; }

	void addNote(uint32_t position, size_t note, uint32_t sustain = 0)
	{
		m_notes[position].set(note, sustain);
	}

	void addPhrase(uint32_t position, SpecialPhrase phrase)
	{
		m_specialPhrases[position].push_back(phrase);
	}

	void addEvent(uint32_t position, std::string_view str)
	{
		if (str[0] == '\"')
			str = str.substr(1, str.length() - 2);
		m_events[position].push_back(str);
	}

	void modifyNote(uint32_t position, char modifier, size_t lane = 0)
	{
		m_notes.at(position).modify(modifier, lane);
	}

	[[nodiscard]] uint32_t getNumActive(uint32_t position)
	{
		return m_notes.at(position).getNumActive();
	}

private:
	void load_V1(TxtFileReader& reader)
	{
		if (isOccupied())
			throw std::runtime_error("Track defined multiple times in file");

		uint32_t solo = 0;

		m_notes.reserve(5000);
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
					addPhrase(position, { SpecialPhraseType::Solo, position - solo });
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

	[[nodiscard]] virtual void load(CommonChartParser* parser) override
	{
		if (isOccupied())
			throw std::runtime_error("Difficulty defined multiple times in file");

		m_notes.reserve(5000);
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
			nodes.try_emplace_back(phrases.key)->m_phrases = &phrases.object;

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

	// Returns whether this difficulty contains notes
	// ONLY checks for notes
	[[nodiscard]] virtual bool hasNotes() const override { return m_notes.size(); }

	// Returns whether this difficulty contains notes, effects, soloes, or other events
	[[nodiscard]] virtual bool isOccupied() const override { return !m_notes.isEmpty() || !m_events.isEmpty() || !m_specialPhrases.isEmpty(); }

	virtual void adjustTicks(float multiplier) override
	{
		for (auto& note : m_notes)
		{
			note.key = uint32_t(note.key * multiplier);
			*note *= multiplier;
		}

		for (auto& vec : m_specialPhrases)
		{
			vec.key = uint32_t(vec.key * multiplier);
			for (auto& special : *vec)
				special *= multiplier;
		}

		for (auto& ev : m_events)
			ev.key = uint32_t(ev.key * multiplier);
	}

	virtual void clear() override
	{
		m_notes.clear();
		m_events.clear();
		m_specialPhrases.clear();
	}

	void shrink()
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}

	T* construct_note_midi(uint32_t position)
	{
		m_notes.reserve(5000);
		return m_notes.try_emplace_back(position);
	}

	void addNote_midi(uint32_t position, size_t note, uint32_t sustain)
	{
		m_notes.getNodeFromBack(position).set(note, sustain);
	}

	std::vector<SpecialPhrase>& getSpecialPhrase_midi(uint32_t position)
	{
		return m_specialPhrases.getNodeFromBack(position);
	}

	T& backNote_midiOnly()
	{
		return m_notes.back();
	}

	T* testBackNote_midiOnly(uint32_t position)
	{
		return m_notes.try_back(position);
	}

private:
	SimpleFlatMap<T> m_notes;
};

