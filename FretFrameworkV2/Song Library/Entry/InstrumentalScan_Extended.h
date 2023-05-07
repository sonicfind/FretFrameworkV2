#pragma once
#include "InstrumentalScan.h"
#include "Serialization/CommonChartParser.h"

namespace InstrumentalScan_Extended
{
	template <class T>
	struct DifficultyTracker
	{
		DifficultyTracker(size_t diff) : m_difficulty(diff) {}
		bool scanDifficulty(ScanValues& values, CommonChartParser& parser)
		{
			if (m_difficulty >= 5 && InstrumentalScan::WasTrackValidated<T>(values, m_difficulty))
				return true;

			while (parser.isStillCurrentTrack())
			{
				const auto trackEvent = parser.parseEvent();
				bool add = false;
				if (trackEvent.second == ChartEvent::NOTE)
				{
					if (testSingleNote(values, parser.extractSingleNote().first))
						return true;
				}
				else if (trackEvent.second == ChartEvent::MULTI_NOTE)
				{
					if (testMultiNote(values, parser.extractMultiNote()))
						return true;
				}
				parser.nextEvent();
			}
			return false;
		}

		[[nodiscard]] bool testSingleNote(ScanValues& values, size_t lane) noexcept
		{
			if (!T::TestIndex(lane))
				return false;
			values.addSubTrack(m_difficulty);
			return true;
		}

		[[nodiscard]] bool testMultiNote(ScanValues& values, const std::vector<std::pair<size_t, uint32_t>>& colors) noexcept
		{
			for (const auto& color : colors)
				if (!T::TestIndex(color.first))
					return false;
			values.addSubTrack(m_difficulty);
			return true;
		}

	private:
		const size_t m_difficulty;
	};

	template <class T>
	bool Scan(ScanValues& values, CommonChartParser& parser)
	{
		if (values.m_subTracks > 0)
			return false;

		while (parser.isStillCurrentTrack())
		{
			if (!parser.isStartOfTrack())
				parser.nextEvent();
			else if (parser.validateDifficultyTrack())
			{
				DifficultyTracker<T> tracker(parser.getDifficulty());
				if (tracker.scanDifficulty(values, parser))
					parser.skipTrack();
			}
			else
				parser.skipTrack();
		}
		return true;
	}
};
