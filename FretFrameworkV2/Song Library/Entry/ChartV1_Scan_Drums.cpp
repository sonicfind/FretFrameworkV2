#include "ChartV1_Scan_Drums.h"

template <>
bool InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(const ScanValues& values, size_t diff)
{
	if (diff < 3)
		return values.wasTrackValidated(diff);
	else
		return values.m_subTracks >= 24;
}

ChartV1_Scan::V1Tracker<DrumNote_Legacy>::V1Tracker(DrumType_Enum type) : m_type(type) {}

bool ChartV1_Scan::V1Tracker<DrumNote_Legacy>::setDifficulty(ScanValues& values, size_t diff)
{
	if (InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(values, diff))
		return false;

	m_difficulty = diff;
	return true;
}

bool ChartV1_Scan::V1Tracker<DrumNote_Legacy>::test(ScanValues& values, size_t note)
{
	if (DrumNote_Legacy::TestIndex_V1(note))
		values.addSubTrack(m_difficulty);

	if (m_type != DrumType_Enum::LEGACY)
		m_type = DrumNote_Legacy::EvaluateDrumType(note);

	return m_type != DrumType_Enum::LEGACY && InstrumentalScan::WasTrackValidated<DrumNote_Legacy>(values, m_difficulty);
}


template<size_t numPads, bool PRO_DRUMS>
bool Test_(ScanValues& values, size_t diff, size_t note)
{
	if (diff < 3)
	{
		if (DrumNote<numPads, PRO_DRUMS>::TestIndex_V1(note))
		{
			values.addSubTrack(diff);
			return true;
		}
		return false;
	}
	else
	{
		if (note == 32)
			values.addSubTrack(4);
		else if (DrumNote<numPads, PRO_DRUMS>::TestIndex_V1(note))
			values.addSubTrack(3);
		return values.m_subTracks >= 24;
	}
}

template <>
bool ChartV1_Scan::V1Tracker<DrumNote<4, true>>::test(ScanValues& values, size_t note) const noexcept
{
	return Test_<4, true>(values, m_difficulty, note);
}

template <>
bool ChartV1_Scan::V1Tracker<DrumNote<5, false>>::test(ScanValues& values, size_t note) const noexcept
{
	return Test_<5, false>(values, m_difficulty, note);
}
