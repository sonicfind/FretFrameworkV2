#include "Song.h"
#include "Tracks/Instrumental/ChartV1_LegacyDrums.h"
#include "Tracks/Instrumental/GuitarForcing.h"
#include "Tracks/Instrumental/DrumTrack_Transfer.h"

void Song::traverse_cht_V1(ChtFileReader& reader)
{
	ChartV1::V1Loader<DrumNote_Legacy> legacy_loader(m_baseDrumType);
	InstrumentalTrack<DrumNote_Legacy> legacy_track;
	while (reader.isStartOfTrack())
	{
		if (reader.validateSyncTrack())
			load_tempoMap(reader);
		else if (reader.validateEventTrack())
		{
			uint64_t phrase = UINT64_MAX;
			while (const auto trackEvent = reader.extractEvent_V1())
			{
				if (trackEvent->second == ChartEvent::EVENT)
				{
					std::string_view str = reader.extractText();
					if (str.starts_with("section "))
						m_events.sections.get_or_emplace_back(trackEvent->first) = str.substr(8);
					else if (str.starts_with("lyric "))
						m_noteTracks.vocals[0][trackEvent->first].lyric = UnicodeString::strToU32(str.substr(6));
					else if (str.starts_with("phrase_start"))
					{
						if (phrase < UINT64_MAX)
							m_noteTracks.vocals.m_specialPhrases[phrase].push_back({ SpecialPhraseType::LyricLine, trackEvent->first - phrase });
						phrase = trackEvent->first;
					}
					else if (str.starts_with("phrase_end"))
					{
						if (phrase < UINT64_MAX)
						{
							m_noteTracks.vocals.m_specialPhrases[phrase].push_back({ SpecialPhraseType::LyricLine, trackEvent->first - phrase });
							phrase = UINT64_MAX;
						}
					}
					else
						m_events.globals.get_or_emplace_back(trackEvent->first).push_back(UnicodeString::strToU32(str));
				}
				reader.nextEvent();
			}
		}
		else
		{
			auto track = reader.extractTrack_V1();
			switch (track)
			{
			case ChtFileReader::Single:
				ChartV1::Load(m_noteTracks.lead_5, reader);
				break;
			case ChtFileReader::DoubleGuitar:
				ChartV1::Load(m_noteTracks.coop, reader);
				break;
			case ChtFileReader::DoubleBass:
				ChartV1::Load(m_noteTracks.bass_5, reader);
				break;
			case ChtFileReader::DoubleRhythm:
				ChartV1::Load(m_noteTracks.rhythm, reader);
				break;
			case ChtFileReader::Drums:
				switch (legacy_loader.getDrumType())
				{
				case DrumType_Enum::LEGACY:       ChartV1::Load(legacy_loader, legacy_track, reader); break;
				case DrumType_Enum::FOURLANE_PRO: ChartV1::Load(m_noteTracks.drums4_pro, reader); break;
				case DrumType_Enum::FIVELANE:     ChartV1::Load(m_noteTracks.drums5, reader); break;
				}
				break;
			case ChtFileReader::Keys:
				ChartV1::Load(m_noteTracks.keys, reader);
				break;
			case ChtFileReader::GHLGuitar:
				ChartV1::Load(m_noteTracks.lead_6, reader);
				break;
			case ChtFileReader::GHLBass:
				ChartV1::Load(m_noteTracks.bass_6, reader);
				break;
			default:
				reader.skipTrack();
			}
		}
	}

	if (legacy_track.isOccupied())
	{
		if (legacy_loader.getDrumType() != DrumType_Enum::FIVELANE)
			LegacyDrums::Transfer(legacy_track, m_noteTracks.drums4_pro);
		else
			LegacyDrums::Transfer(legacy_track, m_noteTracks.drums5);
	}

	const uint64_t hopoThreshold = getHopoThreshold();
	ForcingFix::Fix(m_noteTracks.lead_5, hopoThreshold);
	ForcingFix::Fix(m_noteTracks.lead_6, hopoThreshold);
	ForcingFix::Fix(m_noteTracks.bass_5, hopoThreshold);
	ForcingFix::Fix(m_noteTracks.bass_6, hopoThreshold);
	ForcingFix::Fix(m_noteTracks.rhythm, hopoThreshold);
	ForcingFix::Fix(m_noteTracks.coop, hopoThreshold);
}
