#pragma once
#include "ScanValues.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"
#include "Serialization/CommonChartParser.h"

namespace Extended_Scan_Vocals
{
	template<size_t numTracks>
	bool Scan(ScanValues& values, CommonChartParser& parser)
	{
		if (values.m_subTracks > 0)
			return false;

		VocalPitch pitch;
		uint64_t endOfPhrase = 0;
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
				if (phrase.type == SpecialPhraseType::LyricLine)
					endOfPhrase = trackEvent.first + phrase.getLength();
				break;
			}
			parser.nextEvent();
		}
		return false;
	}
}