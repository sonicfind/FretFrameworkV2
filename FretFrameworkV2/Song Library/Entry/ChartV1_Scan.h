#pragma once
#include "InstrumentScan.h"
#include "Serialization/ChtFileReader.h"

namespace ChartV1_Scan
{
	template <class T>
	class V1Tracker
	{
	public:
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

	private:
		size_t m_difficulty = 0;
	};

	template <class T>
	bool Scan(ScanValues& values, ChtFileReader& reader)
	{
		V1Tracker<T> tracker;
		return Scan(tracker, values, reader);
	}

	template <class T>
	bool Scan(V1Tracker<T>& tracker, ScanValues& values, ChtFileReader& reader)
	{
		if (!tracker.setDifficulty(values, reader.getDifficulty()))
			return false;

		while (const auto chartEvent = reader.extractEvent_V1())
		{
			if (chartEvent->second == ChartEvent::NOTE)
			{
				auto note = reader.extractColorAndSustain_V1();
				if (tracker.test(values, note.first))
					return false;
			}
			reader.nextEvent();
		}
		return true;
	}
};
