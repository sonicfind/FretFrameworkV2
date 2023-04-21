#pragma once
#include "../Track.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"
#include "File Processing/MidiFileReader.h"
#include <assert.h>

template <>
struct Midi_Tracker<Vocal>
{
	const unsigned char starPowerNote;

	uint32_t perc = UINT32_MAX;
	uint32_t vocalPos = UINT32_MAX;
	std::pair<uint32_t, std::string_view> lyric{ UINT32_MAX, "" };

	std::pair<SpecialPhraseType, uint32_t> starPower = { SpecialPhraseType::StarPower,   UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> lyricLine = { SpecialPhraseType::LyricLine,   UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> harmonyLine = { SpecialPhraseType::HarmonyLine, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> rangeShift = { SpecialPhraseType::RangeShift,  UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> lyricShift = { SpecialPhraseType::LyricShift,  UINT32_MAX };

	Midi_Tracker(unsigned char star) : starPowerNote(star) {}
};

template <size_t numTracks>
class VocalTrack : public Track, public BCH_CHT_Extensions
{
public:
	template <size_t INDEX>
	void load(MidiFileReader& reader)
	{
		static_assert(INDEX < numTracks);
		if ((INDEX == 0 && !m_percussion.isEmpty()) || !m_vocals[INDEX].isEmpty())
			throw std::runtime_error("Track defined multiple times in file");

		Midi_Tracker<Vocal> tracker(reader.getStarPowerValue());
		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<INDEX, true>(tracker, note.value, midiEvent->position);
				else
					parseNote<INDEX, false>(tracker, note.value, midiEvent->position);
			}
			else if(midiEvent->type == MidiEventType::Note_Off)
				parseNote<INDEX, false>(tracker, reader.extractMidiNote().value, midiEvent->position);
			else if (midiEvent->type == MidiEventType::SysEx || midiEvent->type == MidiEventType::SysEx_End)
				parseSysEx(reader.extractTextOrSysEx());
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText_midi<INDEX>(tracker.lyric, reader.extractTextOrSysEx(), midiEvent->position);
		}

		if (tracker.lyric.first != UINT32_MAX)
			getVocal_midi<INDEX>(tracker.lyric.first).setLyric(tracker.lyric.second);
	}

	[[nodiscard]] virtual bool hasNotes() const override
	{
		for (const auto& track : m_vocals)
			if (!track.isEmpty())
				return true;

		return !m_percussion.isEmpty();
	}

	[[nodiscard]] virtual bool isOccupied() const override
	{
		return hasNotes() || !m_specialPhrases.isEmpty() || !m_events.isEmpty();
	}

	virtual void shrink() override
	{
		for (auto& track : m_vocals)
			if ((track.size() < 100 || 2000 <= track.size()) && track.size() < track.capacity())
				track.shrink_to_fit();

		if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
			m_percussion.shrink_to_fit();
	}

	virtual void clear() override
	{
		for (auto& track : m_vocals)
			track.clear();
		m_percussion.clear();
		m_events.clear();
		m_specialPhrases.clear();
	}

	virtual void adjustTicks(float multiplier) override
	{
		Track::adjustTicks(multiplier);
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
	}

	[[nodiscard]] Vocal& get_or_emplaceVocal(size_t track, uint32_t position)
	{
		assert(track < numTracks);
		return m_vocals[track][position];
	}

	[[nodiscard]] VocalPercussion& get_or_emplacePercussion(uint32_t position)
	{
		return m_percussion[position];
	}

	[[nodiscard]] Vocal& getVocal(size_t track, uint32_t position)
	{
		assert(track < numTracks);
		return m_vocals[track].at(position);
	}

	[[nodiscard]] VocalPercussion& getPercussion(uint32_t position)
	{
		return m_percussion.at(position);
	}

	[[nodiscard]] const Vocal& getVocal(size_t track, uint32_t position) const
	{
		assert(track < numTracks);
		return m_vocals[track].at(position);
	}

	[[nodiscard]] const VocalPercussion& getPercussion(uint32_t position) const
	{
		return m_percussion.at(position);
	}

	uint32_t getLongestSustain(uint32_t position) const
	{
		uint32_t sustain = 0;
		for (const auto& track : m_vocals)
			if (const Vocal* vocal = track.at_pointer(position))
				if (vocal->getDuration() > sustain)
					sustain = vocal->getDuration();
		return sustain;
	}

	virtual void load(CommonChartParser* parser) override
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
				if (vocal->isPlayable())
				{
					writer->startEvent(position, ChartEvent::VOCAL);
					writer->writeLyric({ i + 1, UnicodeString::U32ToStr(vocal->getLyric()) });
					writer->writePitchAndDuration(vocal->getPitchAndDuration());
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
		writer->setPitchMode(PitchWriteMode::Sharp);

		SimpleFlatMap<VocalWriteNode> nodes;
		size_t vocalSize = 0;
		for (size_t i = 0; i < numTracks; ++i)
			vocalSize += m_vocals[i].size();
		nodes.reserve(vocalSize + m_specialPhrases.size() + m_events.size());

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

private:
	template <size_t INDEX, bool NoteOn>
	void parseNote(Midi_Tracker<Vocal>& tracker, unsigned char midiValue, uint32_t position)
	{
		static constexpr std::pair<unsigned char, unsigned char> PITCHRANGE = { 36, 85 };
		if (PITCHRANGE.first <= midiValue && midiValue < PITCHRANGE.second)
			parseVocal<INDEX, NoteOn>(tracker, midiValue, position);
		else if constexpr (INDEX == 0)
		{
			if (midiValue == 96)
				addPercussion_midi<true, NoteOn>(tracker.perc, position);
			else if (midiValue == 97)
				addPercussion_midi<false, NoteOn>(tracker.perc, position);
			else if (midiValue == 105 || midiValue == 106)
				addSpecialPhrase_midi<NoteOn>(tracker.lyricLine, position);
			else if (midiValue == tracker.starPowerNote)
				addSpecialPhrase_midi<NoteOn>(tracker.starPower, position);
			else if (midiValue == 0)
				addSpecialPhrase_midi<NoteOn>(tracker.rangeShift, position);
			else if (midiValue == 1)
				addSpecialPhrase_midi<NoteOn>(tracker.lyricShift, position);
		}
		else if constexpr (INDEX == 1)
		{
			if (midiValue == 105 || midiValue == 106)
				addSpecialPhrase_midi<NoteOn>(tracker.harmonyLine, position);
		}
	}

	template <size_t INDEX, bool NoteOn>
	void parseVocal(Midi_Tracker<Vocal>& tracker, unsigned char pitch, uint32_t position)
	{
		if (tracker.vocalPos != UINT32_MAX && tracker.lyric.first == tracker.vocalPos)
		{
			uint32_t sustain = position - tracker.vocalPos;
			if constexpr (NoteOn)
			{
				if (sustain > 240)
					sustain -= 120;
				else
					sustain /= 2;
			}

			Vocal& vocal = getVocal_midi<INDEX>(tracker.vocalPos);
			vocal.setLyric(tracker.lyric.second);
			vocal.set(pitch, sustain);
			tracker.lyric.first = UINT32_MAX;
		}

		if constexpr (NoteOn)
			tracker.vocalPos = position;
		else
			tracker.vocalPos = UINT32_MAX;
	}

	template <size_t INDEX>
	Vocal& getVocal_midi(uint32_t position)
	{
		if (m_vocals[INDEX].capacity() == 0)
			m_vocals[INDEX].reserve(500);

		return m_vocals[INDEX].emplace_back(position);
	}

	void parseSysEx(std::string_view sysex)
	{
	}

	template <size_t INDEX>
	void parseText_midi(std::pair<uint32_t, std::string_view>& lyric, std::string_view text, uint32_t position)
	{
		if (text[0] != '[')
		{
			if (lyric.first != UINT32_MAX)
				getVocal_midi<INDEX>(lyric.first).setLyric(lyric.second);
			lyric = { position, text };
		}
		else if constexpr (INDEX == 0)
			m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(text));
	}

	template <bool NoteOn>
	void addSpecialPhrase_midi(std::pair<SpecialPhraseType, uint32_t>& combo, uint32_t position)
	{
		if constexpr (NoteOn)
			combo.second = position;
		else if (combo.second != UINT32_MAX)
		{
			m_specialPhrases.get_or_emplaceNodeFromBack(combo.second).push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	template <bool PLAYABLE, bool NoteOn>
	void addPercussion_midi(uint32_t& percPos, uint32_t position)
	{
		if constexpr (NoteOn)
			percPos = position;
		else if (percPos != UINT32_MAX)
		{
			m_percussion.get_or_emplace_back(percPos).setPlayable(PLAYABLE);
			percPos = UINT32_MAX;
		}
	}

private:
	SimpleFlatMap<Vocal> m_vocals[numTracks];
	SimpleFlatMap<VocalPercussion> m_percussion;
};
