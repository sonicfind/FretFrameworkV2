#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "ChartV1_Scan_Drums.h"

void LibraryEntry::traverse_cht_V1(ChtFileReader& reader)
{
	ChartV1_Scan::V1Tracker<DrumNote_Legacy> legacy_tracker(getDrumTypeFromModifier());
	ScanValues legacy_scanValues;
	while (reader.isStartOfTrack())
	{
		const auto ScanTrack = [&](ChtFileReader::NoteTracks_V1 track) {
			switch (track)
			{
			case ChtFileReader::Single:       return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.lead_5, reader);
			case ChtFileReader::DoubleGuitar: return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.coop, reader);
			case ChtFileReader::DoubleBass:   return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.bass_5, reader);
			case ChtFileReader::DoubleRhythm: return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.rhythm, reader);
			case ChtFileReader::Drums:
				switch (legacy_tracker.getDrumType())
				{
				case DrumType_Enum::LEGACY:       return ChartV1_Scan::Scan(legacy_tracker, legacy_scanValues, reader);
				case DrumType_Enum::FOURLANE_PRO: return ChartV1_Scan::Scan<DrumNote<4, true>>(m_scanTracks.drums4_pro, reader);
				case DrumType_Enum::FIVELANE:     return ChartV1_Scan::Scan<DrumNote<5, false>>(m_scanTracks.drums5, reader);
				}
			case ChtFileReader::Keys:      return ChartV1_Scan::Scan<Keys<5>>(m_scanTracks.keys, reader);
			case ChtFileReader::GHLGuitar: return ChartV1_Scan::Scan<GuitarNote<6>>(m_scanTracks.lead_6, reader);
			case ChtFileReader::GHLBass:   return ChartV1_Scan::Scan<GuitarNote<6>>(m_scanTracks.bass_6, reader);
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
