#pragma once
#include "VocalTrack.h"
#include "Serialization/CommonChartParser.h"

namespace Extended_Load_Vocals
{
	template <size_t numTracks>
	bool Load(VocalTrack<numTracks>& track, CommonChartParser& parser)
	{
		if (track.isOccupied())
			return false;

		for (auto& vocals : track.m_vocals)
			vocals.reserve(1000);
		track.m_percussion.reserve(200);

		while (parser.isStillCurrentTrack())
		{
			const auto trackEvent = parser.parseEvent();
			switch (trackEvent.second)
			{
			case ChartEvent::LYRIC:
			case ChartEvent::VOCAL:
			{
				auto lyric = parser.extractLyric();
				if (lyric.first == 0 || lyric.first > numTracks)
					throw std::runtime_error("Invalid track index");

				Vocal& vocal = track.m_vocals[lyric.first - 1].get_or_emplace_back(trackEvent.first);
				vocal.setLyric(lyric.second);

				if (trackEvent.second == ChartEvent::VOCAL)
				{
					auto values = parser.extractPitchAndDuration();
					if (!vocal.set(values.first, values.second))
						throw std::runtime_error("Invalid pitch");
				}
				break;
			}
			case ChartEvent::VOCAL_PERCUSSION:
			{
				auto& perc = track.m_percussion.get_or_emplace_back(trackEvent.first);
				const auto& modifiers = parser.extractSingleNoteMods();
				for (const auto mod : modifiers)
					perc.modify(mod);
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = parser.extractSpecialPhrase();
				switch (phrase.getType())
				{
				case SpecialPhraseType::LyricShift:
					phrase.setDuration(1);
					__fallthrough;
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::LyricLine:
				case SpecialPhraseType::RangeShift:
				case SpecialPhraseType::HarmonyLine:
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
};
