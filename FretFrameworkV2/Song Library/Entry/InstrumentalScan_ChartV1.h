#pragma once
#include "InstrumentalScan.h"
#include "Serialization/ChtFileReader.h"

namespace InstrumentalScan_ChartV1
{
	template <class T>
	struct V1Tracker
	{
		size_t m_difficulty = 0;
		[[nodiscard]] bool setDifficulty(ScanValues& values, size_t diff)
		{
			if (InstrumentalScan::WasTrackValidated<T>(values, diff))
				return false;

			m_difficulty = diff;
			return true;
		}

		[[nodiscard]] bool test(ScanValues& values, size_t note) const noexcept
		{
			if (T::TestIndex_V1(note))
			{
				values.addSubTrack(m_difficulty);
				return true;
			}
			return false;
		}
	};

	template <class T>
	void Scan(V1Tracker<T>& tracker, ScanValues& values, ChtFileReader& reader)
	{
		if (!tracker.setDifficulty(values, reader.getDifficulty()))
			return;

		while (reader.isStillCurrentTrack())
		{
			const auto chartEvent = reader.parseEvent();
			if (chartEvent.second == ChartEvent::NOTE)
			{
				auto note = reader.extractColorAndSustain_V1();
				if (tracker.test(values, note.first))
				{
					reader.skipTrack();
					return;
				}
			}
			reader.nextEvent();
		}
	}
};
