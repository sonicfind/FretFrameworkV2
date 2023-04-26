#include "CacheEntry.h"
#include "Serialization/ChtFileWriter.h"
#include "Legacy_DrumScan.h"

void CacheEntry::scan_cht_V1(TxtFileReader& reader)
{
	Legacy_DrumScan drumsLegacy;
	while (reader.isStartOfTrack())
	{
		auto track = reader.extractTrack_V1();
		switch (track)
		{
		case TxtFileReader::Single:
			m_noteTracks.lead_5.scan_V1(reader.getDifficulty(), reader);
			break;
		case TxtFileReader::DoubleGuitar:
			m_noteTracks.coop.scan_V1(reader.getDifficulty(), reader);
			break;
		case TxtFileReader::DoubleBass:
			m_noteTracks.bass_5.scan_V1(reader.getDifficulty(), reader);
			break;
		case TxtFileReader::DoubleRhythm:
			m_noteTracks.rhythm.scan_V1(reader.getDifficulty(), reader);
			break;
		case TxtFileReader::Drums:
			switch (drumsLegacy.getDrumType())
			{
			case DrumType_Enum::LEGACY:       drumsLegacy.scan_V1(reader.getDifficulty(), reader); break;
			case DrumType_Enum::FOURLANE_PRO: m_noteTracks.drums4_pro.scan_V1(reader.getDifficulty(), reader); break;
			case DrumType_Enum::FIVELANE:     m_noteTracks.drums5.scan_V1(reader.getDifficulty(), reader); break;
			}
			break;
		case TxtFileReader::Keys:
			m_noteTracks.keys.scan_V1(reader.getDifficulty(), reader);
			break;
		case TxtFileReader::GHLGuitar:
			m_noteTracks.lead_6.scan_V1(reader.getDifficulty(), reader);
			break;
		case TxtFileReader::GHLBass:
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
