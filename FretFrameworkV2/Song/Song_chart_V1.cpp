#include "Song.h"
#include "Tracks/Instrumental/ChartV1.h"
#include "Tracks/Instrumental/GuitarForcing.h"
#include "Tracks/Instrumental/DrumTrack_Transfer.h"

void Song::traverse_cht_V1(ChtFileReader& reader)
{
	InstrumentalTrack<DrumNote_Legacy> legacy_track;
	const auto loadNoteTrack = [&] (std::pair<ChtFileReader::NoteTracks_V1, size_t> track) {
		switch (track.first)
		{
			case ChtFileReader::Single:       return ChartV1::Load(m_noteTracks.lead_5[track.second], reader);
			case ChtFileReader::DoubleGuitar: return ChartV1::Load(m_noteTracks.coop[track.second], reader);
			case ChtFileReader::DoubleBass:   return ChartV1::Load(m_noteTracks.bass_5[track.second], reader);
			case ChtFileReader::DoubleRhythm: return ChartV1::Load(m_noteTracks.rhythm[track.second], reader);
			case ChtFileReader::Drums:
				switch (DrumNote_Legacy::GetType())
				{
					case DrumType_Enum::FOURLANE_PRO: return ChartV1::Load(m_noteTracks.drums4_pro[track.second], reader);
					case DrumType_Enum::FIVELANE:     return ChartV1::Load(m_noteTracks.drums5[track.second], reader);
					default:                          return ChartV1::Load(legacy_track[track.second], reader);
				}
			case ChtFileReader::Keys:      return ChartV1::Load(m_noteTracks.keys[track.second], reader);
			case ChtFileReader::GHLGuitar: return ChartV1::Load(m_noteTracks.lead_6[track.second], reader);
			case ChtFileReader::GHLBass:   return ChartV1::Load(m_noteTracks.bass_6[track.second], reader);
			default:
				return false;
		}
	};

	DrumNote_Legacy::ResetType();
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
		else if (!loadNoteTrack(reader.extractTrackAndDifficulty_V1()))
			reader.skipTrack();
	}

	if (legacy_track.isOccupied())
	{
		if (DrumNote_Legacy::GetType() != DrumType_Enum::FIVELANE)
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
