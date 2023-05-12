#pragma once
#include "InstrumentScan.h"
#include "Serialization/CommonChartParser.h"

namespace Extended_Scan
{
	template <class T>
	[[nodiscard]] bool TestSingleNote(InstrumentScan<T>& scan, const size_t diff, size_t lane) noexcept
	{
		if (!T::TestIndex(lane))
			return false;
		scan.addDifficulty(diff);
		return true;
	}

	template <class T>
	[[nodiscard]] bool TestMultiNote(InstrumentScan<T>& scan, const size_t diff, const std::vector<std::pair<size_t, uint64_t>>& colors) noexcept
	{
		for (const auto& color : colors)
			if (!T::TestIndex(color.first))
				return false;
		scan.addDifficulty(diff);
		return true;
	}

	template <class T>
	[[nodiscard]] bool Scan(InstrumentScan<T>& scan, CommonChartParser& parser)
	{
		if (scan.getSubTracks() > 0)
			return false;

		auto scanDifficulty = [&] {
			if (!parser.validateDifficultyTrack())
				return false;

			const size_t diff = parser.getDifficulty();
			if (diff >= 5 || scan.hasSubTrack(diff))
				return false;

			while (parser.isStillCurrentTrack())
			{
				const auto trackEvent = parser.parseEvent();
				if (trackEvent.second == ChartEvent::NOTE)
				{
					if (TestSingleNote(scan, diff, parser.extractSingleNote().first))
						return false;
				}
				else if (trackEvent.second == ChartEvent::MULTI_NOTE)
				{
					if (TestMultiNote(scan, diff, parser.extractMultiNote()))
						return false;
				}
				parser.nextEvent();
			}
			return true;
		};

		while (parser.isStillCurrentTrack())
		{
			if (!parser.isStartOfTrack())
				parser.nextEvent();
			else if (!scanDifficulty())
				parser.skipTrack();
		}
		return true;
	}
};
