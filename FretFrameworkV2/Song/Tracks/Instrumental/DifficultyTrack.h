#pragma once
#include "../Track.h"
#include "Serialization/ChtFileReader.h"
#include "Serialization/MidiFileWriter.h"

template <typename T, bool EXTENDED>
class InstrumentalTrack;

template <typename T>
class InstrumentalTrack_Extended;

template <typename T, bool EXTENDED>
class DifficultyTrack : public Track
{
	friend InstrumentalTrack<T, true>;
	friend InstrumentalTrack<T, false>;
public:
	SimpleFlatMap<T> m_notes;

public:
	virtual void adjustTicks(float multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& note : m_notes)
		{
			note.key = uint32_t(note.key * multiplier);
			*note *= multiplier;
		}
	}

	virtual void clear() override
	{
		Track::clear();
		m_notes.clear();
	}

	[[nodiscard]] virtual bool hasNotes() const override { return m_notes.size(); }
	[[nodiscard]] virtual bool isOccupied() const override { return !m_notes.isEmpty() || Track::isOccupied(); }

	[[nodiscard]] T& at(uint32_t position)
	{
		return m_notes.at(position);
	}

	[[nodiscard]] const T& at(uint32_t position) const
	{
		return m_notes.at(position);
	}

	T& operator[](size_t position)
	{
		return m_notes[position];
	}

protected:
	virtual void shrink() override
	{
		if ((m_notes.size() < 500 || 10000 <= m_notes.size()) && m_notes.size() < m_notes.capacity())
			m_notes.shrink_to_fit();
	}

private:
	void load_V1(ChtFileReader& reader)
	{
		if (isOccupied())
			throw std::runtime_error("Track defined multiple times in file");

		uint32_t solo = 0;
		m_notes.reserve(5000);
		while (reader.isStillCurrentTrack())
		{
			const auto trackEvent = reader.parseEvent();
			switch (trackEvent.second)
			{
			case ChartEvent::NOTE:
			{
				auto note = reader.extractColorAndSustain_V1();
				if (!m_notes.get_or_emplace_back(trackEvent.first).set_V1(note.first, note.second))
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
					m_specialPhrases.get_or_emplace_back(trackEvent.first).push_back(phrase);
				}
				break;
			}
			case ChartEvent::EVENT:
			{
				std::string_view str = reader.extractText();
				if (str.starts_with("soloend"))
					m_specialPhrases[trackEvent.first].push_back({ SpecialPhraseType::Solo, trackEvent.first - solo });
				else if (str.starts_with("solo"))
					solo = trackEvent.first;
				else
					m_events.get_or_emplace_back(trackEvent.first).push_back(UnicodeString::strToU32(str));
				break;
			}
			}
			reader.nextEvent();
		}

		shrink();
	}

	PointerWrapper<T> construct_note_midi(uint32_t position)
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

	template <unsigned char INDEX>
	void save_events_phrases(MidiFileWriter& writer, bool& doPhrases) const
	{
		static constexpr char OFFSET = 60 + 12 * INDEX;
		if (doPhrases)
		{
			for (const auto& vec : m_specialPhrases)
			{
				for (const auto& phrase : *vec)
				{
					switch (phrase.getType())
					{
					case SpecialPhraseType::StarPower:
						writer.addMidiNote(vec.key, 116, 100, phrase.getDuration());
						doPhrases = false;
						break;
					case SpecialPhraseType::StarPowerActivation:
						writer.addMidiNote(vec.key, 120, 100, phrase.getDuration());
						writer.addMidiNote(vec.key, 121, 100, phrase.getDuration());
						writer.addMidiNote(vec.key, 122, 100, phrase.getDuration());
						writer.addMidiNote(vec.key, 123, 100, phrase.getDuration());
						writer.addMidiNote(vec.key, 124, 100, phrase.getDuration());
						doPhrases = false;
						break;
					case SpecialPhraseType::Solo:
						writer.addMidiNote(vec.key, 103, 100, phrase.getDuration());
						doPhrases = false;
						break;
					case SpecialPhraseType::Tremolo:
						writer.addMidiNote(vec.key, 126, 100, phrase.getDuration());
						doPhrases = false;
						break;
					case SpecialPhraseType::Trill:
						writer.addMidiNote(vec.key, 127, 100, phrase.getDuration());
						doPhrases = false;
						break;
					case SpecialPhraseType::StarPower_Diff:
						writer.addMidiNote(vec.key, OFFSET + 7, 100, phrase.getDuration());
						break;
					case SpecialPhraseType::FaceOff_Player1:
						writer.addMidiNote(vec.key, OFFSET + 9, 100, phrase.getDuration());
						break;
					case SpecialPhraseType::FaceOff_Player2:
						writer.addMidiNote(vec.key, OFFSET + 10, 100, phrase.getDuration());
						break;
					}
				}
			}
		}
		write_details<INDEX>(writer);
	}

	template <unsigned char INDEX>
	void save_notes(MidiFileWriter& writer) const
	{
		static constexpr char OFFSET = 60 + 12 * INDEX;
		for (const auto& note : m_notes)
			for (const auto& col : note->getMidiNotes())
				writer.addMidiNote(note.key, OFFSET + std::get<0>(col), std::get<1>(col), std::get<2>(col));
	}

	template <unsigned char INDEX>
	void write_details(MidiFileWriter& writer) const {}
};

template <typename T>
class DifficultyTrack<T, true> : public DifficultyTrack<T, false>, public BCH_CHT_Extensions
{
	friend InstrumentalTrack<T, false>;
	friend InstrumentalTrack<T, true>;
	friend InstrumentalTrack_Extended;
public:
	using DifficultyTrack<T, false>::m_notes;
	using DifficultyTrack<T, false>::m_specialPhrases;
	using DifficultyTrack<T, false>::m_events;

private:
	virtual void load(CommonChartParser& parser) override
	{
		if (this->isOccupied())
			throw std::runtime_error("Difficulty defined multiple times in file");

		m_notes.reserve(5000);
		while (parser.isStillCurrentTrack())
		{
			const auto trackEvent = parser.parseEvent();
			switch (trackEvent.second)
			{
			case ChartEvent::NOTE:
			{
				auto& note = m_notes.get_or_emplace_back(trackEvent.first);
				const auto color = parser.extractSingleNote();
				if (!note.set(color.first, color.second))
					throw std::runtime_error("Note color is invalid");

				const auto& modifiers = parser.extractSingleNoteMods();
				for (const auto mod : modifiers)
					note.modify(mod, color.first);
				break;
			}
			case ChartEvent::MULTI_NOTE:
			{
				auto& note = m_notes.get_or_emplace_back(trackEvent.first);
				const auto colors = parser.extractMultiNote();
				for (const auto& color : colors)
					if (!note.set(color.first, color.second))
						throw std::runtime_error("Note color is invalid");
				break;
			}
			case ChartEvent::MODIFIER:
			{
				auto& note = m_notes.back(trackEvent.first);

				const auto& modifiers = parser.extractMultiNoteMods();
				for (const auto& node : modifiers)
					note.modify(node.first, node.second);
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = parser.extractSpecialPhrase();
				switch (phrase.getType())
				{
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::Solo:
				case SpecialPhraseType::StarPowerActivation:
				case SpecialPhraseType::Tremolo:
				case SpecialPhraseType::Trill:
					m_specialPhrases.get_or_emplace_back(trackEvent.first).push_back(phrase);
				}
				break;
			}
			case ChartEvent::EVENT:
			{
				auto& events = m_events.get_or_emplace_back(trackEvent.first);
				events.push_back(UnicodeString::strToU32(parser.extractText()));
				break;
			}
			}
			parser.nextEvent();
		}
		this->shrink();
	}

	struct DiffWriteNode : public WriteNode
	{
		const T* m_note = nullptr;

		void writeNote(const uint32_t position, CommonChartWriter& writer) const
		{
			if (m_note == nullptr)
				return;

			std::vector<std::pair<size_t, uint32_t>> colors = m_note->getActiveColors();
			if (colors.size() == 1)
			{
				writer.startEvent(position, ChartEvent::NOTE);
				writer.writeSingleNote(colors[0]);
				writer.writeSingleNoteMods(m_note->getActiveModifiers(colors[0].first));
				writer.finishEvent();
			}
			else if (colors.size() > 1)
			{
				writer.startEvent(position, ChartEvent::MULTI_NOTE);
				writer.writeMultiNote(colors);
				writer.finishEvent();

				const std::vector<std::pair<char, size_t>> modifiers = m_note->getActiveModifiers();
				if (!modifiers.empty())
				{
					writer.startEvent(position, ChartEvent::MODIFIER);
					writer.writeMultiNoteMods(modifiers);
					writer.finishEvent();
				}
			}
		}
	};

	virtual void save(CommonChartWriter& writer) const override
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
};

