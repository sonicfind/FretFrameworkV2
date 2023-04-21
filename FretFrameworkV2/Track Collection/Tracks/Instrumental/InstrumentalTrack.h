#pragma once
#include <iostream>
#include "DifficultyTrack.h"
#include "File Processing/MidiFileReader.h"
#include "Midi_Tracker.h"

template <class T>
class InstrumentalTrack : public Track
{
public:
	DifficultyTrack<T> m_difficulties[5];

public:
	InstrumentalTrack() = default;
	InstrumentalTrack(InstrumentalTrack&&) = default;
	InstrumentalTrack& operator=(InstrumentalTrack&&) = default;

	InstrumentalTrack(const InstrumentalTrack&) = delete;
	InstrumentalTrack& operator=(const InstrumentalTrack&) = delete;

	void load_V1(size_t diff, TxtFileReader& reader)
	{
		m_difficulties[diff].load_V1(reader);
	}

	void load(MidiFileReader& reader)
	{
		if (isOccupied())
			throw std::runtime_error("Track defined multiple times in file");

		Midi_Tracker<T> tracker(reader.getStarPowerValue());
		while (auto midiEvent = reader.parseEvent())
		{
			if (midiEvent->type == MidiEventType::Note_On)
			{
				MidiNote note = reader.extractMidiNote();
				if (note.velocity > 0)
					parseNote<true>(tracker, note, midiEvent->position);
				else
					parseNote<false>(tracker, note, midiEvent->position);
			}
			else if (midiEvent->type == MidiEventType::Note_Off)
				parseNote<false>(tracker, reader.extractMidiNote(), midiEvent->position);
			else if (midiEvent->type == MidiEventType::SysEx || midiEvent->type == MidiEventType::SysEx_End)
				parseSysEx(tracker, reader.extractTextOrSysEx(), midiEvent->position);
			else if (midiEvent->type <= MidiEventType::Text_EnumLimit)
				parseText(tracker, reader.extractTextOrSysEx(), midiEvent->position);
		}
	}

	[[nodiscard]] virtual bool hasNotes() const override
	{
		for (const auto& diff : m_difficulties)
			if (diff.hasNotes())
				return true;
		return false;
	}

	[[nodiscard]] virtual bool isOccupied() const override
	{
		for (const auto& diff : m_difficulties)
			if (diff.isOccupied())
				return true;
		return Track::isOccupied();
	}

	virtual void clear() override
	{
		Track::clear();
		for (auto& diff : m_difficulties)
			diff.clear();
	}

	virtual void shrink() override
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

	[[nodiscard]] T& get_or_emplaceNote(size_t diffIndex, uint32_t position)
	{
		return m_difficulties[diffIndex].get_or_emplaceNote(position);
	}

	[[nodiscard]] std::vector<SpecialPhrase>& get_or_emplacePhrases(size_t diffIndex, uint32_t position)
	{
		return m_difficulties[diffIndex].get_or_emplacePhrases(position);
	}

	[[nodiscard]] std::vector<std::u32string>& get_or_emplaceEvents(size_t diffIndex, uint32_t position)
	{
		return m_difficulties[diffIndex].get_or_emplaceEvents(position);
	}

	[[nodiscard]] const T& getNote(size_t diffIndex, uint32_t position) const
	{
		return m_difficulties[diffIndex].getNote(position);
	}

	virtual void adjustTicks(float multiplier)
	{
		Track::adjustTicks(multiplier);
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);
	}

private:
	static constexpr std::pair<unsigned char, unsigned char> s_noteRange{ 60, 100 };
	static constexpr int s_diffValues[48] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
	};

	template <bool NoteOn>
	void parseNote(Midi_Tracker<T>& tracker, MidiNote note, uint32_t position)
	{
		if (s_noteRange.first <= note.value && note.value <= s_noteRange.second)
			parseLaneColor<NoteOn>(tracker, note, position);
		else if (120 <= note.value && note.value <= 124)
			parseBRE<NoteOn>(tracker, note.value, position);
		else if (note.value == tracker.starPowerNote)
			addSpecialPhrase<NoteOn>(tracker.starPower, position);
		else if (note.value == 103)
			addSpecialPhrase<NoteOn>(tracker.solo, position);
		else if (note.value == 126)
			addSpecialPhrase<NoteOn>(tracker.tremolo, position);
		else if (note.value == 127)
			addSpecialPhrase<NoteOn>(tracker.trill, position);
		else
			toggleExtraValues<NoteOn>(tracker, note, position);
	}

	template <bool NoteOn>
	void parseLaneColor(Midi_Tracker<T>& tracker, MidiNote note, uint32_t position)
	{
		const int noteValue = note.value - s_noteRange.first;
		const int lane = tracker.laneValues[noteValue];
		const int diff = s_diffValues[noteValue];

		if (lane < T::GetLaneCount())
		{
			if constexpr (NoteOn)
			{
				m_difficulties[diff].construct_note_midi(position);
				tracker.difficulties[diff].notes[lane] = position;
			}
			else
			{
				uint32_t colorPosition = tracker.difficulties[diff].notes[lane];
				if (colorPosition != UINT32_MAX)
				{
					m_difficulties[diff].m_notes.getNodeFromBack(colorPosition).set(lane + 1, position - colorPosition);
					tracker.difficulties[diff].notes[lane] = UINT32_MAX;
				}
			}
		}
	}

	template <bool NoteOn>
	void parseBRE(Midi_Tracker<T>& tracker, uint32_t midiValue, uint32_t position)
	{
		if constexpr (NoteOn)
		{
			tracker.notes_BRE[midiValue - 120] = position;
			tracker.doBRE = tracker.notes_BRE[0] == tracker.notes_BRE[1] && tracker.notes_BRE[1] == tracker.notes_BRE[2] && tracker.notes_BRE[2] == tracker.notes_BRE[3];
		}
		else if (tracker.doBRE)
		{
			uint32_t colorPosition = tracker.notes_BRE[0];
			m_specialPhrases[colorPosition].push_back({ SpecialPhraseType::StarPowerActivation, position - colorPosition });

			for (size_t i = 0; i < 5; ++i)
				tracker.notes_BRE[i] = UINT32_MAX;
			tracker.doBRE = false;
		}
		else
		{
			const int lane = midiValue - 120;
			uint32_t colorPosition = tracker.notes_BRE[lane];
			if (colorPosition != UINT32_MAX)
			{
				m_difficulties[4].m_notes[colorPosition].set(lane + 1, position - colorPosition);
				tracker.notes_BRE[lane] = UINT32_MAX;
			}
		}
	}

	void parseSysEx(Midi_Tracker<T>& tracker, std::string_view str, uint32_t position) {}
	void parseText(Midi_Tracker<T>& tracker, std::string_view str, uint32_t position)
	{
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
	}

	template <bool NoteOn>
	void toggleExtraValues(Midi_Tracker<T>& tracker, MidiNote note, uint32_t position) {}

	template <bool NoteOn>
	void addSpecialPhrase(std::pair<SpecialPhraseType, uint32_t>& combo, uint32_t position)
	{
		if constexpr (NoteOn)
		{
			m_specialPhrases.try_emplace_back(position);
			combo.second = position;
		}
		else if (combo.second != UINT32_MAX)
		{
			m_specialPhrases.getNodeFromBack(combo.second).push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}

	template <bool NoteOn>
	void addSpecialPhrase(std::pair<SpecialPhraseType, uint32_t>& combo, size_t diff, uint32_t position)
	{
		if constexpr (NoteOn)
		{
			m_difficulties[diff].m_specialPhrases.try_emplace_back(position);
			combo.second = position;
		}
		else if (combo.second != UINT32_MAX)
		{
			m_difficulties[diff].m_specialPhrases.getNodeFromBack(combo.second).push_back({ combo.first, position - combo.second });
			combo.second = UINT32_MAX;
		}
	}
};

template <class T>
class InstrumentalTrack_Extended : public InstrumentalTrack<T>, public BCH_CHT_Extensions
{
public:
	using InstrumentalTrack<T>::InstrumentalTrack;
	using InstrumentalTrack<T>::load;
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
						this->m_difficulties[diff].load(parser);
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
			if (this->m_difficulties[i].isOccupied())
			{
				writer->writeDifficultyTrack(i);
				this->m_difficulties[i].save(writer);
				writer->finishTrack();
			}
	}

protected:
	virtual void parse_event(CommonChartParser* parser)
	{
		const uint32_t position = parser->parsePosition();
		switch (parser->parseEvent())
		{
		case ChartEvent::SPECIAL:
		{
			auto& phrases = this->m_specialPhrases.get_or_emplace_back(position);
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
			auto& events = this->m_events.get_or_emplace_back(position);
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
		SimpleFlatMap<WriteNode> nodes(this->m_specialPhrases.size() + this->m_events.size());
		for (const auto& note : this->m_specialPhrases)
			nodes.emplace_back(note.key).m_phrases = &note.object;

		for (const auto& events : this->m_events)
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
};
