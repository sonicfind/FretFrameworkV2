#pragma once
#include "InstrumentalTrack.h"
#include "Serialization/CommonChartParser.h"

namespace Extended_Load_Instrument
{
	template <class T>
	bool Load(DifficultyTrack<T>& track, CommonChartParser& parser)
	{
		if (track.isOccupied())
			return false;

		track.m_notes.reserve(5000);
		while (parser.isStillCurrentTrack())
		{
			const auto trackEvent = parser.parseEvent();
			switch (trackEvent.second)
			{
			case ChartEvent::NOTE:
			{
				auto& note = track.m_notes.get_or_emplace_back(trackEvent.first);
				const auto color = parser.extractSingleNote();
				if (!note.set(color.first, color.second))
				{
					if (!track.m_notes.back().validate())
						track.m_notes.pop_back();
					break;
				}

				const auto& modifiers = parser.extractSingleNoteMods();
				for (const auto mod : modifiers)
					note.modify(mod, color.first);
				break;
			}
			case ChartEvent::MULTI_NOTE:
			{
				auto& note = track.m_notes.get_or_emplace_back(trackEvent.first);
				const auto colors = parser.extractMultiNote();
				for (const auto& color : colors)
				{
					if (!note.set(color.first, color.second))
					{
						if (!track.m_notes.back().validate())
							track.m_notes.pop_back();
						break;
					}
				}
				break;
			}
			case ChartEvent::MODIFIER:
			{
				auto& note = track.m_notes.back(trackEvent.first);

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
					track.m_specialPhrases.get_or_emplace_back(trackEvent.first).push_back(phrase);
				}
				break;
			}
			case ChartEvent::EVENT:
			{
				auto& events = track.m_events.get_or_emplace_back(trackEvent.first);
				events.push_back(UnicodeString::strToU32(parser.extractText()));
				break;
			}
			}
			parser.nextEvent();
		}
		track.shrink();
		return true;
	}

	template <class T>
	void ParseEvent(InstrumentalTrack<T>& track, CommonChartParser& parser)
	{
		const auto trackEvent = parser.parseEvent();
		switch (trackEvent.second)
		{
		case ChartEvent::SPECIAL:
		{
			auto& phrases = track.m_specialPhrases.get_or_emplace_back(trackEvent.first);
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
			auto& events = track.m_events.get_or_emplace_back(trackEvent.first);
			events.push_back(UnicodeString::strToU32(parser.extractText()));
			break;
		}
		}
		parser.nextEvent();
	}

	template <class T>
	bool Load(InstrumentalTrack<T>& track, CommonChartParser& parser)
	{
		if (track.isOccupied())
			return false;

		while (parser.isStillCurrentTrack())
		{
			if (!parser.isStartOfTrack())
				ParseEvent(track, parser);
			else if (!parser.validateDifficultyTrack() || !Load(track[parser.getDifficulty()], parser))
				parser.skipTrack();
		}
		return true;
	}
};
