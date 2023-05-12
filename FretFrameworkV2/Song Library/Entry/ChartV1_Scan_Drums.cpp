#include "ChartV1_Scan_Drums.h"

template<class DrumType, size_t numPads>
bool Test_(InstrumentScan<DrumNote<DrumType, numPads>>& values, size_t diff, size_t note)
{
	if (diff < 3)
	{
		if (DrumNote<DrumType, numPads>::TestIndex_V1(note))
		{
			values.addDifficulty(diff);
			return true;
		}
		return false;
	}
	else
	{
		if (note == 32)
			values.addDifficulty(4);
		else if (DrumNote<DrumType, numPads>::TestIndex_V1(note))
			values.addDifficulty(3);
		return values.getSubTracks() >= 24;
	}
}

template <>
bool ChartV1_Scan::Test<DrumNote<DrumPad_Pro, 4>>(InstrumentScan<DrumNote<DrumPad_Pro, 4>>& scan, size_t diff, size_t note)
{
	return Test_(scan, diff, note);
}

template <>
bool ChartV1_Scan::Test<DrumNote<DrumPad, 5>>(InstrumentScan<DrumNote<DrumPad, 5>>& scan, size_t diff, size_t note)
{
	return Test_(scan, diff, note);
}

template <>
bool ChartV1_Scan::Test<DrumNote_Legacy>(InstrumentScan<DrumNote_Legacy>& scan, size_t diff, size_t note)
{
	scan.setType(DrumNote_Legacy::EvaluateDrumType(note));
	return Test_(scan, diff, note) && scan.getType() != DrumType_Enum::LEGACY;
}
