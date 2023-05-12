#pragma once
#include "DifficultyTrack.h"

namespace ChartV1
{
	template <class T>
	[[nodiscard]] bool Load(DifficultyTrack<T>& diff, ChtFileReader& reader)
	{
		if (diff.isOccupied())
			return false;

		uint64_t solo = 0;
		diff.m_notes.reserve(5000);
		while (const auto trackEvent = reader.extractEvent_V1())
		{
			switch (trackEvent->second)
			{
			case ChartEvent::NOTE:
			{
				const auto note = reader.extractColorAndSustain_V1();
				if (!diff.m_notes.get_or_emplace_back(trackEvent->first).set_V1(note.first, note.second))
					if (!diff.m_notes.back().validate())
						diff.m_notes.pop_back();
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = reader.extractSpecialPhrase();
				switch (phrase.type)
				{
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::StarPowerActivation:
				case SpecialPhraseType::Tremolo:
				case SpecialPhraseType::Trill:
					diff.m_specialPhrases.get_or_emplace_back(trackEvent->first).push_back(phrase);
				}
				break;
			}
			case ChartEvent::EVENT:
			{
				std::string_view str = reader.extractText();
				if (str.starts_with("soloend"))
					diff.m_specialPhrases[trackEvent->first].push_back({ SpecialPhraseType::Solo, trackEvent->first - solo });
				else if (str.starts_with("solo"))
					solo = trackEvent->first;
				else
					diff.m_events.get_or_emplace_back(trackEvent->first).push_back(UnicodeString::strToU32(str));
				break;
			}
			}
			reader.nextEvent();
		}
		diff.shrink();
		return true;
	}
};
