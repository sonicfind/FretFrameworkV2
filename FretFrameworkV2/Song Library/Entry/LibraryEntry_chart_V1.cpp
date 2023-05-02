#include "LibraryEntry.h"
#include "Legacy_DrumScan.h"

void LibraryEntry::traverse_cht_V1(ChtFileReader& reader)
{
	Legacy_DrumScan drumsLegacy = [&]() -> Legacy_DrumScan {
		if (auto fivelane = getModifier("five_lane_drums"))
			return fivelane->getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;
		return DrumType_Enum::LEGACY;
	}();

	while (reader.isStartOfTrack())
	{
		auto track = reader.extractTrack_V1();
		switch (track)
		{
		case ChtFileReader::Single:
			m_scanTracks.lead_5.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::DoubleGuitar:
			m_scanTracks.coop.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::DoubleBass:
			m_scanTracks.bass_5.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::DoubleRhythm:
			m_scanTracks.rhythm.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::Drums:
			switch (drumsLegacy.getDrumType())
			{
			case DrumType_Enum::LEGACY:       drumsLegacy.scan_V1(reader.getDifficulty(), reader); break;
			case DrumType_Enum::FOURLANE_PRO: m_scanTracks.drums4_pro.scan_V1(reader.getDifficulty(), reader); break;
			case DrumType_Enum::FIVELANE:     m_scanTracks.drums5.scan_V1(reader.getDifficulty(), reader); break;
			}
			break;
		case ChtFileReader::Keys:
			m_scanTracks.keys.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::GHLGuitar:
			m_scanTracks.lead_6.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::GHLBass:
			m_scanTracks.bass_6.scan_V1(reader.getDifficulty(), reader);
			break;
		default:
			reader.skipTrack();
		}
	}

	if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
		drumsLegacy.transfer(m_scanTracks.drums4_pro);
	else
		drumsLegacy.transfer(m_scanTracks.drums5);
}
