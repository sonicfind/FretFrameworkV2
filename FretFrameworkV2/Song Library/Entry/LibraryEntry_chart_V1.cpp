#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "DrumScan_ChartV1.h"
#include "DrumScan_Legacy_ChartV1.h"

void LibraryEntry::traverse_cht_V1(ChtFileReader& reader)
{
	InstrumentalScan_ChartV1::V1Tracker<GuitarNote<5>>      lead_5;
	InstrumentalScan_ChartV1::V1Tracker<GuitarNote<6>>      lead_6;
	InstrumentalScan_ChartV1::V1Tracker<GuitarNote<5>>      bass_5;
	InstrumentalScan_ChartV1::V1Tracker<GuitarNote<6>>      bass_6;
	InstrumentalScan_ChartV1::V1Tracker<GuitarNote<5>>      coop;
	InstrumentalScan_ChartV1::V1Tracker<GuitarNote<5>>      rhythm;
	InstrumentalScan_ChartV1::V1Tracker<Keys<5>>            keys;
	InstrumentalScan_ChartV1::V1Tracker<DrumNote<4, true>>  drums4_pro;
	InstrumentalScan_ChartV1::V1Tracker<DrumNote<5, false>> drums5;
	InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy>    drums_legacy([&]{
		if (auto fivelane = getModifier("five_lane_drums"))
			return fivelane->getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;
		return DrumType_Enum::LEGACY;
	}());

	while (reader.isStartOfTrack())
	{
		auto track = reader.extractTrack_V1();
		switch (track)
		{
		case ChtFileReader::Single:
			InstrumentalScan_ChartV1::Scan(lead_5, m_scanTracks.lead_5, reader);
			break;
		case ChtFileReader::DoubleGuitar:
			InstrumentalScan_ChartV1::Scan(coop, m_scanTracks.coop, reader);
			break;
		case ChtFileReader::DoubleBass:
			InstrumentalScan_ChartV1::Scan(bass_5, m_scanTracks.bass_5, reader);
			break;
		case ChtFileReader::DoubleRhythm:
			InstrumentalScan_ChartV1::Scan(rhythm, m_scanTracks.rhythm, reader);
			break;
		case ChtFileReader::Drums:
			switch (drums_legacy.getDrumType())
			{
			case DrumType_Enum::LEGACY:       InstrumentalScan_ChartV1::Scan(drums_legacy, drums_legacy.m_values, reader); break;
			case DrumType_Enum::FOURLANE_PRO: InstrumentalScan_ChartV1::Scan(drums4_pro, m_scanTracks.drums4_pro, reader); break;
			case DrumType_Enum::FIVELANE:     InstrumentalScan_ChartV1::Scan(drums5, m_scanTracks.drums5, reader); break;
			}
			break;
		case ChtFileReader::Keys:
			InstrumentalScan_ChartV1::Scan(keys, m_scanTracks.keys, reader);
			break;
		case ChtFileReader::GHLGuitar:
			InstrumentalScan_ChartV1::Scan(lead_6, m_scanTracks.lead_6, reader);
			break;
		case ChtFileReader::GHLBass:
			InstrumentalScan_ChartV1::Scan(bass_6, m_scanTracks.bass_6, reader);
			break;
		default:
			reader.skipTrack();
		}
	}

	if (drums_legacy.getDrumType() != DrumType_Enum::FIVELANE)
		drums_legacy.transfer(m_scanTracks.drums4_pro);
	else
		drums_legacy.transfer(m_scanTracks.drums5);
}
