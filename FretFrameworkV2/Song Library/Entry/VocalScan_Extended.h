#pragma once
#include "ScanValues.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"
#include "Serialization/CommonChartParser.h"

namespace VocalScan_Extended
{
	template<size_t numTracks>
	bool Scan(ScanValues& values, CommonChartParser& parser)
	{
		if (values.m_subTracks > 0)
			return false;

		VocalPitch pitch;
		uint32_t endOfPhrase = 0;
		parser.nextEvent();
		while (parser.isStillCurrentTrack())
		{
			const auto trackEvent = parser.parseEvent();
			if (trackEvent.second == ChartEvent::VOCAL)
			{
				if (trackEvent.first >= endOfPhrase)
					break;

				auto lyric = parser.extractLyric();
				if (lyric.first == 0 || lyric.first > numTracks || values.wasTrackValidated(lyric.first))
					break;

				auto pitchValues = parser.extractPitchAndDuration();
				if (pitch.set(pitchValues.first))
				{
					values.addSubTrack(lyric.first);
					if (values.m_subTracks == (1 << numTracks) - 1)
						return false;
				}
				break;
			}
			else if (trackEvent.second == ChartEvent::SPECIAL)
			{
				auto phrase = parser.extractSpecialPhrase();
				if (phrase.getType() == SpecialPhraseType::LyricLine)
					endOfPhrase = trackEvent.first + phrase.getDuration();
				break;
			}
			parser.nextEvent();
		}
		return false;
	}
}
