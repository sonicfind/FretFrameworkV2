#pragma once
#include "InstrumentScan.h"
#include "Serialization/ChtFileReader.h"

namespace ChartV1_Scan
{
	template <class T>
	bool Test(InstrumentScan<T>& scan, size_t diff, size_t note)
	{
		if (T::TestIndex_V1(note))
		{
			scan.addDifficulty(diff);
			return true;
		}
		return false;
	}

	template <class T>
	bool Scan(InstrumentScan<T>& scan, const size_t diff, ChtFileReader& reader)
	{
		if (scan.hasSubTrack(diff))
			return false;

		while (const auto chartEvent = reader.extractEvent_V1())
		{
			if (chartEvent->second == ChartEvent::NOTE)
			{
				auto note = reader.extractColorAndSustain_V1();
				if (Test(scan, diff, note.first))
					return false;
			}
			reader.nextEvent();
		}
		return true;
	}
};
