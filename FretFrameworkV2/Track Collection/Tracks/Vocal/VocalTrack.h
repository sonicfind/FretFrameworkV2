#pragma once
#include "../Track.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"

template <size_t numTracks>
class VocalTrackMidiParser;

template <size_t numTracks>
class VocalTrack : public Track
{
	friend class VocalTrackMidiParser<numTracks>;
public:
	[[nodiscard]] virtual void load(CommonChartParser* parser) override
	{
		if (isOccupied())
		{
			if constexpr (numTracks == 1)
				throw std::runtime_error("Vocal track defined multiple times in file");
			else
				throw std::runtime_error("Harmony track defined multiple times in file");
		}

		for (auto& track : m_vocals)
			track.reserve(1000);
		m_percussion.reserve(200);

		parser->nextEvent();
		while (parser->isStillCurrentTrack())
		{
			const uint32_t position = parser->parsePosition();
			const ChartEvent ev = parser->parseEvent();
			switch (ev)
			{
			case ChartEvent::LYRIC:
			case ChartEvent::VOCAL:
			{
				auto lyric = parser->extractLyric();
				if (lyric.first == 0 || lyric.first > numTracks)
					throw std::runtime_error("Invalid track index");

				Vocal& vocal = m_vocals[lyric.first - 1].get_or_emplace_back(position);
				vocal.setLyric(lyric.second);

				if (ev == ChartEvent::VOCAL)
				{
					auto values = parser->extractPitchAndDuration();
					if (!vocal.set(values.first, values.second))
						throw std::runtime_error("Invalid pitch");
				}
				break;
			}
			case ChartEvent::VOCAL_PERCUSSION:
			{
				auto& perc = m_percussion.get_or_emplace_back(position);
				const auto& modifiers = parser->extractSingleNoteMods();
				for (const auto mod : modifiers)
					perc.modify(mod);
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = parser->extractSpecialPhrase();
				switch (phrase.getType())
				{
				case SpecialPhraseType::LyricShift:
					phrase.setDuration(1);
					__fallthrough;
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::LyricLine:
				case SpecialPhraseType::RangeShift:
				case SpecialPhraseType::HarmonyLine:
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

	struct VocalWriteNode : public WriteNode
	{
		const Vocal* m_vocals[numTracks] = {};
		const VocalPercussion* m_perc = nullptr;

		void writeVocals(const uint32_t position, CommonChartWriter* writer) const
		{
			for (size_t i = 0; i < numTracks; ++i)
			{
				if (m_vocals[i] == nullptr)
					continue;

				const Vocal* const vocal = m_vocals[i];

				std::pair<char, uint32_t> values = vocal->getPitchAndDuration();
				if (values.first || values.second)
				{
					writer->startEvent(position, ChartEvent::VOCAL);
					writer->writeLyric({ i + 1, UnicodeString::U32ToStr(vocal->getLyric()) });
					writer->writePitchAndDuration(values);
				}
				else
				{
					writer->startEvent(position, ChartEvent::LYRIC);
					writer->writeLyric({ i + 1, UnicodeString::U32ToStr(vocal->getLyric()) });
				}
				writer->finishEvent();
			}
		}

		void writePercussion(const uint32_t position, CommonChartWriter* writer) const
		{
			if (m_perc == nullptr)
				return;

			writer->startEvent(position, ChartEvent::VOCAL_PERCUSSION);
			writer->writeSingleNoteMods(m_perc->getActiveModifiers());
			writer->finishEvent();
		}
	};

	virtual void save(CommonChartWriter* writer) const override
	{
		size_t vocalSize = 0;
		for (size_t i = 0; i < numTracks; ++i)
			vocalSize += m_vocals[i].size();

		SimpleFlatMap<VocalWriteNode> nodes(vocalSize + m_specialPhrases.size() + m_events.size());
		for (const auto& phrases : m_specialPhrases)
			nodes.emplace_back(phrases.key).m_phrases = &phrases.object;

		for (size_t i = 0; i < numTracks; ++i)
			for (const auto& vocal : m_vocals[i])
				nodes[vocal.key].m_vocals[i] = &vocal.object;

		for (const auto& perc : m_percussion)
			nodes[perc.key].m_perc = &perc.object;

		for (const auto& events : m_events)
			nodes[events.key].m_events = &events.object;

		for (const auto& node : nodes)
		{
			node->writePhrases(node.key, writer);
			node->writeVocals(node.key, writer);
			node->writePercussion(node.key, writer);
			node->writeEvents(node.key, writer);
		}
	}

	// Returns whether this track contains any notes
	// ONLY checks for notes
	[[nodiscard]] bool hasNotes() const override
	{
		for (const auto& track : m_vocals)
			if (!track.isEmpty())
				return true;

		return !m_percussion.isEmpty();
	}

	template <size_t INDEX>
	[[nodiscard]] bool hasNotes() const
	{
		static_assert(INDEX <= numTracks);
		if constexpr (INDEX == 0)
			return !m_percussion.isEmpty() && !m_vocals[INDEX].isEmpty();
		return !m_vocals[INDEX].isEmpty();
	}

	// Returns whether this track contains any notes, special phrases, soloes, or other events
	[[nodiscard]] bool isOccupied() const override
	{
		return hasNotes() || !m_specialPhrases.isEmpty() || !m_events.isEmpty();
	}

	void clear() override
	{
		for (auto& track : m_vocals)
			track.clear();
		m_percussion.clear();
		m_events.clear();
		m_specialPhrases.clear();
	}

	void adjustTicks(float multiplier) override
	{
		for (auto& track : m_vocals)
		{
			for (auto& vocal : track)
			{
				vocal.key = uint32_t(vocal.key * multiplier);
				*vocal *= multiplier;
			}
		}

		for (auto& perc : m_percussion)
			perc.key = uint32_t(perc.key * multiplier);

		for (auto& vec : m_specialPhrases)
		{
			vec.key = uint32_t(vec.key * multiplier);
			for (SpecialPhrase& phrase : *vec)
				phrase *= multiplier;
		}

		for (auto& ev : m_events)
			ev.key = uint32_t(ev.key * multiplier);
	}

	[[nodiscard]] int save_midi(std::fstream& outFile) const;

	void addLyric(int lane, uint32_t position, const std::u32string& lyric)
	{
		m_vocals[lane][position].setLyric(lyric);
	}

	void addLyric(int lane, uint32_t position, std::string_view lyric)
	{
		m_vocals[lane][position].setLyric(lyric);
	}

	void addPercussion(uint32_t position, bool playable)
	{
		VocalPercussion& perc = m_percussion[position];
		if (!playable)
			perc.setPlayable(true);
	}

	void addPhrase(uint32_t position, SpecialPhrase phrase)
	{
		m_specialPhrases[position].push_back(phrase);
	}

	void addEvent(uint32_t position, std::string_view ev)
	{
		m_events[position].push_back(UnicodeString::strToU32(ev));
	}

	void setPitch(int lane, uint32_t position, char pitch, uint32_t duration = 0)
	{
		m_vocals[lane][position].set(pitch, duration);
	}

	void shrink()
	{
		for (auto& track : m_vocals)
			if ((track.size() < 100 || 2000 <= track.size()) && track.size() < track.capacity())
				track.shrink_to_fit();

		if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
			m_percussion.shrink_to_fit();
	}

private:
	template<int index>
	void save_midi(const std::string& name, std::fstream& outFile) const;

	uint32_t getLongestSustain(uint32_t position) const
	{
		uint32_t sustain = 0;
		for (const auto& track : m_vocals)
			if (const Vocal* vocal = track.at_pointer(position))
				if (vocal->getDuration() > sustain)
					sustain = vocal->getDuration();
		return sustain;
	}

	template <size_t INDEX>
	Vocal& getVocal_midi(uint32_t position)
	{
		if (m_vocals[INDEX].capacity() == 0)
			m_vocals[INDEX].reserve(500);

		return m_vocals[INDEX].emplace_back(position);
	}

	VocalPercussion& getPercusssion_midi(uint32_t position)
	{
		return m_percussion.get_or_emplace_back(position);
	}

	std::vector<std::u32string>& get_or_emplace_Events_midi(uint32_t position)
	{
		return m_events.get_or_emplace_back(position);
	}

	std::vector<SpecialPhrase>& get_or_emplace_SpecialPhrase_midi(uint32_t position)
	{
		return m_specialPhrases.get_or_emplaceNodeFromBack(position);
	}

	template <size_t INDEX>
	Vocal* testBackNote_midiOnly(uint32_t position)
	{
		return m_vocals[INDEX].try_back(position);
	}

	template <size_t INDEX>
	void shrink_midi()
	{
		static_assert(INDEX < numTracks);
		if ((m_vocals[INDEX].size() < 100 || 2000 <= m_vocals[INDEX].size()) && m_vocals[INDEX].size() < m_vocals[INDEX].capacity())
			m_vocals[INDEX].shrink_to_fit();

		if constexpr (INDEX == 0)
			if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
				m_percussion.shrink_to_fit();
	}

private:
	SimpleFlatMap<Vocal> m_vocals[numTracks];
	SimpleFlatMap<VocalPercussion> m_percussion;
};
