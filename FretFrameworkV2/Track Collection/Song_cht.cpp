#include "Song.h"
#include "File Processing/ChtFileWriter.h"

void Song::load_cht(const std::filesystem::path& path)
{
	TxtFileReader reader(path);
	InstrumentalTrack<DrumNote_Legacy> drumsLegacy;

	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	const int version = load_songInfo_cht(reader);
	while (reader.isStartOfTrack())
	{
		if (!load_tempoMap(&reader))
		{
			if ((version > 1  && !load_events(&reader) && !load_noteTrack(&reader)) ||
				(version <= 1 && !load_events_V1(reader) && !load_noteTrack_V1(reader, drumsLegacy)))
			{
				reader.skipUnknownTrack();
			}
		}
	}

	if (drumsLegacy.isOccupied())
	{
		if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
			m_noteTracks.drums4_pro = std::move(drumsLegacy);
		else
			m_noteTracks.drums5 = std::move(drumsLegacy);
	}
}

void Song::save_cht(const std::filesystem::path& path)
{
	ChtFileWriter writer(path);
	save(&writer);
}

int Song::load_songInfo_cht(TxtFileReader& reader)
{
	int version = 0;
	reader.nextEvent();
	while (reader.isStillCurrentTrack())
	{
		const std::string_view modifier = reader.parseModifierName();
		if (modifier == "Resolution")
			m_tickrate = reader.extract<uint32_t>();
		else if (modifier == "FileVersion")
			version = reader.extract<uint32_t>();
		reader.nextEvent();
	}
	return version;
}

bool Song::load_events_V1(TxtFileReader& reader)
{
	if (!reader.validateEventTrack())
		return false;

	uint32_t phrase = UINT32_MAX;
	reader.nextEvent();
	while (reader.isStillCurrentTrack())
	{
		const uint32_t position = reader.parsePosition();
		if (reader.parseEvent() == ChartEvent::EVENT)
		{
			std::string_view str = reader.extractText();
			if (str.starts_with("section "))
				m_sectionMarkers.get_or_emplace_back(position) = str.substr(8);
			else if (str.starts_with("lyric "))
				m_noteTracks.vocals.get_or_emplaceVocal(0, position).setLyric(str.substr(6));
			else if (str.starts_with("phrase_start"))
			{
				if (phrase < UINT32_MAX)
					m_noteTracks.vocals.get_or_emplacePhrases(phrase).push_back({ SpecialPhraseType::LyricLine, position - phrase });
				phrase = position;
			}
			else if (str.starts_with("phrase_end"))
			{
				if (phrase < UINT32_MAX)
				{
					m_noteTracks.vocals.get_or_emplacePhrases(phrase).push_back({ SpecialPhraseType::LyricLine, position - phrase });
					phrase = UINT32_MAX;
				}
			}
			else
				m_globalEvents.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
		}
		reader.nextEvent();
	}
	return true;
}

bool Song::load_noteTrack_V1(TxtFileReader& reader, InstrumentalTrack<DrumNote_Legacy>& drumsLegacy)
{
	auto track = reader.extractTrack_V1();
	if (track == TxtFileReader::Invalid)
		return false;

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
	}
	return true;
}
