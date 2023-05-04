#pragma once
#include "DifficultyTrack.h"
#include "Serialization/MidiFileReader.h"

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

	void load_V1(size_t diff, ChtFileReader& reader)
	{
		m_difficulties[diff].load_V1(reader);
	}

private:
	struct Midi_Tracker_Diff
	{
		uint32_t notes[T::GetLaneCount()];
		constexpr Midi_Tracker_Diff() { for (uint32_t& note : notes) note = UINT32_MAX; }
	};

	struct Midi_Tracker_Extensions {};

	struct Midi_Tracker
	{
		static constexpr int s_defaultLanes[48] =
		{
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		};

		const unsigned char starPowerNote;

		int laneValues[48];
		uint32_t notes_BRE[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		bool doBRE = false;

		std::pair<SpecialPhraseType, uint32_t> solo = { SpecialPhraseType::Solo, UINT32_MAX };
		std::pair<SpecialPhraseType, uint32_t> starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
		std::pair<SpecialPhraseType, uint32_t> tremolo = { SpecialPhraseType::Tremolo, UINT32_MAX };
		std::pair<SpecialPhraseType, uint32_t> trill = { SpecialPhraseType::Trill, UINT32_MAX };

		Midi_Tracker_Diff difficulties[4];
		Midi_Tracker_Extensions ext;

		Midi_Tracker(unsigned char star) : starPowerNote(star)
		{
			memcpy(laneValues, s_defaultLanes, sizeof(s_defaultLanes));
		}
	};

public:
	void load(MidiFileReader& reader)
	{
		if (isOccupied())
			throw std::runtime_error("Track defined multiple times in file");

		Midi_Tracker tracker(reader.getMultiplierNote());
		while (const auto midiEvent = reader.parseEvent())
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

	void save(MidiFileWriter& writer, std::string_view name) const
	{
		if (!isOccupied())
			return;

		writer.setTrackName(name);
		writeMidiToggleEvent(writer);

		for (const auto& vec : m_events)
			for (const auto& ev : *vec)
				writer.addText(vec.key, UnicodeString::U32ToStr(ev));

		bool doPhrases_diff = m_specialPhrases.isEmpty();
		for (const auto& vec : m_specialPhrases)
		{
			for (const auto& phrase : *vec)
			{
				switch (phrase.getType())
				{
				case SpecialPhraseType::StarPower:
					writer.addMidiNote(vec.key, 116, 100, phrase.getDuration());
					break;
				case SpecialPhraseType::StarPowerActivation:
					writer.addMidiNote(vec.key, 120, 100, phrase.getDuration());
					writer.addMidiNote(vec.key, 121, 100, phrase.getDuration());
					writer.addMidiNote(vec.key, 122, 100, phrase.getDuration());
					writer.addMidiNote(vec.key, 123, 100, phrase.getDuration());
					writer.addMidiNote(vec.key, 124, 100, phrase.getDuration());
					break;
				case SpecialPhraseType::Solo:
					writer.addMidiNote(vec.key, 103, 100, phrase.getDuration());
					break;
				case SpecialPhraseType::Tremolo:
					writer.addMidiNote(vec.key, 126, 100, phrase.getDuration());
					break;
				case SpecialPhraseType::Trill:
					writer.addMidiNote(vec.key, 127, 100, phrase.getDuration());
					break;
				}
			}
		}

		for (unsigned char i = 0; i < 4; i++)
			m_difficulties[i].save(writer, i, doPhrases_diff);

		doPhrases_diff = false;
		m_difficulties[4].save(writer, 5, doPhrases_diff);
		writer.writeTrack();
	}

public:
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

	DifficultyTrack<T>& operator[](size_t i)
	{
		return m_difficulties[i];
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
	void parseNote(Midi_Tracker& tracker, MidiNote note, uint32_t position)
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
	void parseLaneColor(Midi_Tracker& tracker, MidiNote note, uint32_t position)
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
	void parseBRE(Midi_Tracker& tracker, uint32_t midiValue, uint32_t position)
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

	void parseSysEx(Midi_Tracker& tracker, std::string_view str, uint32_t position) {}
	void parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
	{
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
	}

	template <bool NoteOn>
	void toggleExtraValues(Midi_Tracker& tracker, MidiNote note, uint32_t position) {}

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

private:
	void writeMidiToggleEvent(MidiFileWriter& writer) const {}
};

template <class T>
class InstrumentalTrack_Extended : public InstrumentalTrack<T>, public BCH_CHT_Extensions
{
public:
	using InstrumentalTrack<T>::InstrumentalTrack;
	using InstrumentalTrack<T>::load;
	using InstrumentalTrack<T>::save;
	virtual void load(CommonChartParser& parser) override
	{
		while (parser.isStillCurrentTrack())
		{
			if (!parser.isStartOfTrack())
				parse_event(parser);
			else
			{
				if (parser.validateDifficultyTrack())
				{
					const size_t diff = parser.getDifficulty();
					if (diff < 5)
						this->m_difficulties[diff].load(parser);
					else // BCH only
						parser.skipTrack();

				}
				else if (parser.validateAnimationTrack())
					load_anim(parser);
				else
					parser.skipTrack();
			}
		}
	}

	virtual void save(CommonChartWriter& writer) const override
	{
		save_events(writer);
		save_anim(writer);
		for (unsigned char i = 0; i < 5; ++i)
			if (this->m_difficulties[i].isOccupied())
			{
				writer.writeDifficultyTrack(i);
				this->m_difficulties[i].save(writer);
				writer.finishTrack();
			}
	}

protected:
	virtual void parse_event(CommonChartParser& parser)
	{
		const auto trackEvent = parser.parseEvent();
		switch (trackEvent.second)
		{
		case ChartEvent::SPECIAL:
		{
			auto& phrases = this->m_specialPhrases.get_or_emplace_back(trackEvent.first);
			auto phrase = parser.extractSpecialPhrase();
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
			auto& events = this->m_events.get_or_emplace_back(trackEvent.first);
			events.push_back(UnicodeString::strToU32(parser.extractText()));
			break;
		}
		}
		parser.nextEvent();
	}

	virtual void load_anim(CommonChartParser& parser)
	{
		while (parser.isStillCurrentTrack())
		{
			const auto trackEvent = parser.parseEvent();
			parser.nextEvent();
		}
	}

	virtual void save_events(CommonChartWriter& writer) const
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

	virtual void save_anim(CommonChartWriter& parser) const
	{
	}
};
