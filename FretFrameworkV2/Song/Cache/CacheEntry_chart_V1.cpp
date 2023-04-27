#include "CacheEntry.h"
#include "Legacy_DrumScan.h"

void CacheEntry::scan_cht_V1(ChtFileReader& reader)
{
	Legacy_DrumScan drumsLegacy;
	while (reader.isStartOfTrack())
	{
		auto track = reader.extractTrack_V1();
		switch (track)
		{
		case ChtFileReader::Single:
			m_noteTracks.lead_5.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::DoubleGuitar:
			m_noteTracks.coop.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::DoubleBass:
			m_noteTracks.bass_5.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::DoubleRhythm:
			m_noteTracks.rhythm.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::Drums:
			switch (drumsLegacy.getDrumType())
			{
			case DrumType_Enum::LEGACY:       drumsLegacy.scan_V1(reader.getDifficulty(), reader); break;
			case DrumType_Enum::FOURLANE_PRO: m_noteTracks.drums4_pro.scan_V1(reader.getDifficulty(), reader); break;
			case DrumType_Enum::FIVELANE:     m_noteTracks.drums5.scan_V1(reader.getDifficulty(), reader); break;
			}
			break;
		case ChtFileReader::Keys:
			m_noteTracks.keys.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::GHLGuitar:
			m_noteTracks.lead_6.scan_V1(reader.getDifficulty(), reader);
			break;
		case ChtFileReader::GHLBass:
			m_noteTracks.bass_6.scan_V1(reader.getDifficulty(), reader);
			break;
		default:
			reader.skipTrack();
		}
	}

	if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
		drumsLegacy.transfer(m_noteTracks.drums4_pro);
	else
		drumsLegacy.transfer(m_noteTracks.drums5);
}
