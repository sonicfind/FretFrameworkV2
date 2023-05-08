#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "DrumScan_ChartV1.h"
#include "DrumScan_Legacy_ChartV1.h"

void LibraryEntry::traverse_cht_V1(ChtFileReader& reader)
{
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
			InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.lead_5, reader);
			break;
		case ChtFileReader::DoubleGuitar:
			InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.coop, reader);
			break;
		case ChtFileReader::DoubleBass:
			InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.bass_5, reader);
			break;
		case ChtFileReader::DoubleRhythm:
			InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.rhythm, reader);
			break;
		case ChtFileReader::Drums:
			switch (drums_legacy.getDrumType())
			{
			case DrumType_Enum::LEGACY:       InstrumentalScan_ChartV1::Scan(drums_legacy, drums_legacy.m_values, reader); break;
			case DrumType_Enum::FOURLANE_PRO: InstrumentalScan_ChartV1::Scan<DrumNote<4, true>>(m_scanTracks.drums4_pro, reader); break;
			case DrumType_Enum::FIVELANE:     InstrumentalScan_ChartV1::Scan<DrumNote<5, false>>(m_scanTracks.drums5, reader); break;
			}
			break;
		case ChtFileReader::Keys:
			InstrumentalScan_ChartV1::Scan<Keys<5>>(m_scanTracks.keys, reader);
			break;
		case ChtFileReader::GHLGuitar:
			InstrumentalScan_ChartV1::Scan<GuitarNote<6>>(m_scanTracks.lead_6, reader);
			break;
		case ChtFileReader::GHLBass:
			InstrumentalScan_ChartV1::Scan<GuitarNote<6>>(m_scanTracks.bass_6, reader);
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
