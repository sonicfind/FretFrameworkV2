#include "DrumScan_Legacy_ChartV1.h"

InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy>::V1Tracker(DrumType_Enum type) : DrumScan_Legacy(type) {}

bool InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy>::setDifficulty(ScanValues& values, size_t diff)
{
	if (InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(values, diff))
		return false;

	m_difficulty = diff;
	return true;
}

bool InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy>::test(ScanValues& values, size_t note)
{
	if (DrumNote_Legacy::TestIndex_V1(note))
		values.addSubTrack(m_difficulty);

	if (m_type != DrumType_Enum::LEGACY)
		m_type = DrumNote_Legacy::EvaluateDrumType(note);

	return m_type != DrumType_Enum::LEGACY && InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(values, m_difficulty);
}
