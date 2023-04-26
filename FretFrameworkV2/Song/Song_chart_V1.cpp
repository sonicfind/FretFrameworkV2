#include "Song.h"
#include "Serialization/ChtFileWriter.h"
#include "Tracks/Instrumental/Legacy_DrumTrack.h"

void Song::load_cht_V1(TxtFileReader& reader)
{
	Legacy_DrumTrack drumsLegacy;
	while (reader.isStartOfTrack())
	{
		if (reader.validateSyncTrack())
			load_tempoMap(reader);
		else if (reader.validateEventTrack())
			load_events_V1(reader);
		else
		{
			auto track = reader.extractTrack_V1();
			switch (track)
			{
			case TxtFileReader::Single:
				m_noteTracks.lead_5.load_V1(reader.getDifficulty(), reader);
				break;
			case TxtFileReader::DoubleGuitar:
				m_noteTracks.coop.load_V1(reader.getDifficulty(), reader);
				break;
			case TxtFileReader::DoubleBass:
				m_noteTracks.bass_5.load_V1(reader.getDifficulty(), reader);
				break;
			case TxtFileReader::DoubleRhythm:
				m_noteTracks.rhythm.load_V1(reader.getDifficulty(), reader);
				break;
			case TxtFileReader::Drums:
				switch (drumsLegacy.getDrumType())
				{
				case DrumType_Enum::LEGACY:       drumsLegacy.load_V1(reader.getDifficulty(), reader); break;
				case DrumType_Enum::FOURLANE_PRO: m_noteTracks.drums4_pro.load_V1(reader.getDifficulty(), reader); break;
				case DrumType_Enum::FIVELANE:     m_noteTracks.drums5.load_V1(reader.getDifficulty(), reader); break;
				}
				break;
			case TxtFileReader::Keys:
				m_noteTracks.keys.load_V1(reader.getDifficulty(), reader);
				break;
			case TxtFileReader::GHLGuitar:
				m_noteTracks.lead_6.load_V1(reader.getDifficulty(), reader);
				break;
			case TxtFileReader::GHLBass:
				m_noteTracks.bass_6.load_V1(reader.getDifficulty(), reader);
				break;
			default:
				reader.skipTrack();
			}
		}
	}

	if (drumsLegacy.isOccupied())
	{
		if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
			drumsLegacy.transfer(m_noteTracks.drums4_pro);
		else
			drumsLegacy.transfer(m_noteTracks.drums5);
	}
}

void Song::load_events_V1(TxtFileReader& reader)
{
	uint32_t phrase = UINT32_MAX;
	while (reader.isStillCurrentTrack())
	{
		const auto trackEvent = reader.parseEvent();
		if (trackEvent.second == ChartEvent::EVENT)
		{
			std::string_view str = reader.extractText();
			if (str.starts_with("section "))
				m_sectionMarkers.get_or_emplace_back(trackEvent.first) = str.substr(8);
			else if (str.starts_with("lyric "))
				m_noteTracks.vocals[0][trackEvent.first].setLyric(str.substr(6));
			else if (str.starts_with("phrase_start"))
			{
				if (phrase < UINT32_MAX)
					m_noteTracks.vocals.m_specialPhrases[phrase].push_back({ SpecialPhraseType::LyricLine, trackEvent.first - phrase });
				phrase = trackEvent.first;
			}
			else if (str.starts_with("phrase_end"))
			{
				if (phrase < UINT32_MAX)
				{
					m_noteTracks.vocals.m_specialPhrases[phrase].push_back({ SpecialPhraseType::LyricLine, trackEvent.first - phrase });
					phrase = UINT32_MAX;
				}
			}
			else
				m_globalEvents.get_or_emplace_back(trackEvent.first).push_back(UnicodeString::strToU32(str));
		}
		reader.nextEvent();
	}
}
