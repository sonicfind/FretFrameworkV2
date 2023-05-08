#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "DrumScan_ChartV1.h"

void LibraryEntry::traverse_cht_V1(ChtFileReader& reader)
{
	InstrumentalScan_ChartV1::V1Tracker<DrumNote_Legacy> legacy_tracker(getDrumTypeFromModifier());
	ScanValues legacy_scanValues;
	while (reader.isStartOfTrack())
	{
		const auto ScanTrack = [&](ChtFileReader::NoteTracks_V1 track) {
			switch (track)
			{
			case ChtFileReader::Single:       return InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.lead_5, reader);
			case ChtFileReader::DoubleGuitar: return InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.coop, reader);
			case ChtFileReader::DoubleBass:   return InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.bass_5, reader);
			case ChtFileReader::DoubleRhythm: return InstrumentalScan_ChartV1::Scan<GuitarNote<5>>(m_scanTracks.rhythm, reader);
			case ChtFileReader::Drums:
				switch (legacy_tracker.getDrumType())
				{
				case DrumType_Enum::LEGACY:       return InstrumentalScan_ChartV1::Scan(legacy_tracker, legacy_scanValues, reader);
				case DrumType_Enum::FOURLANE_PRO: return InstrumentalScan_ChartV1::Scan<DrumNote<4, true>>(m_scanTracks.drums4_pro, reader);
				case DrumType_Enum::FIVELANE:     return InstrumentalScan_ChartV1::Scan<DrumNote<5, false>>(m_scanTracks.drums5, reader);
				}
			case ChtFileReader::Keys:      return InstrumentalScan_ChartV1::Scan<Keys<5>>(m_scanTracks.keys, reader);
			case ChtFileReader::GHLGuitar: return InstrumentalScan_ChartV1::Scan<GuitarNote<6>>(m_scanTracks.lead_6, reader);
			case ChtFileReader::GHLBass:   return InstrumentalScan_ChartV1::Scan<GuitarNote<6>>(m_scanTracks.bass_6, reader);
			default:
				return false;
			}
		};

		auto track = reader.extractTrack_V1();
		if (!ScanTrack(track))
			reader.skipTrack();
	}

	if (legacy_tracker.getDrumType() != DrumType_Enum::FIVELANE)
		m_scanTracks.drums4_pro.m_subTracks |= legacy_scanValues.m_subTracks;
	else
		m_scanTracks.drums5.m_subTracks |= legacy_scanValues.m_subTracks;
}
