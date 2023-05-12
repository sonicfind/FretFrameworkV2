#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "ChartV1_Scan_Drums.h"

void LibraryEntry::traverse_cht_V1(ChtFileReader& reader)
{
	InstrumentScan<DrumNote_Legacy> legacyScans(getDrumTypeFromModifier());
	while (reader.isStartOfTrack())
	{
		const auto ScanTrack = [&](std::pair<ChtFileReader::NoteTracks_V1, size_t> track) {
			switch (track.first)
			{
			case ChtFileReader::Single:       return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.lead_5, track.second, reader);
			case ChtFileReader::DoubleGuitar: return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.coop, track.second, reader);
			case ChtFileReader::DoubleBass:   return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.bass_5, track.second, reader);
			case ChtFileReader::DoubleRhythm: return ChartV1_Scan::Scan<GuitarNote<5>>(m_scanTracks.rhythm, track.second, reader);
			case ChtFileReader::Drums:
				switch (legacyScans.getType())
				{
				case DrumType_Enum::LEGACY:       return ChartV1_Scan::Scan(legacyScans, track.second, reader);
				case DrumType_Enum::FOURLANE_PRO: return ChartV1_Scan::Scan<DrumNote<DrumPad_Pro, 4>>(m_scanTracks.drums4_pro, track.second, reader);
				case DrumType_Enum::FIVELANE:     return ChartV1_Scan::Scan<DrumNote<DrumPad, 5>>(m_scanTracks.drums5, track.second, reader);
				}
			case ChtFileReader::Keys:      return ChartV1_Scan::Scan<Keys<5>>(m_scanTracks.keys, track.second, reader);
			case ChtFileReader::GHLGuitar: return ChartV1_Scan::Scan<GuitarNote<6>>(m_scanTracks.lead_6, track.second, reader);
			case ChtFileReader::GHLBass:   return ChartV1_Scan::Scan<GuitarNote<6>>(m_scanTracks.bass_6, track.second, reader);
			default:
				return false;
			}
		};

		if (!ScanTrack(reader.extractTrackAndDifficulty_V1()))
			reader.skipTrack();
	}

	if (legacyScans.getType() != DrumType_Enum::FIVELANE)
		m_scanTracks.drums4_pro = legacyScans;
	else
		m_scanTracks.drums5 = legacyScans;
}
