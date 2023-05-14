#include "Song.h"
#include <iostream>

Song::Song(const std::filesystem::path& directory) : m_directory(directory) {}
Song::Song(const std::filesystem::path& directory,
	       const std::u32string& name,
	       const std::u32string& artist,
	       const std::u32string& album,
	       const std::u32string& genre,
	       const std::u32string& year,
	       const std::u32string& charter,
	       const std::u32string& playlist,
	       const uint64_t hopoFrequency,
	       const uint16_t hopofreq_old,
	       const unsigned char multiplierNote,
	       const bool eighthNoteHopo,
	       const uint64_t sustainCutoffThreshold,
	       const DrumType_Enum drumType)
	: m_directory(directory)
	, m_name(name), m_artist(artist), m_album(album), m_genre(genre), m_year(year), m_charter(charter), m_playlist(playlist)
	, m_hopo_frequency(hopoFrequency), m_hopofreq_old(hopofreq_old), m_multiplier_note(multiplierNote)
	, m_eighthnote_hopo(eighthNoteHopo), m_sustain_cutoff_threshold(sustainCutoffThreshold), m_baseDrumType(drumType) {}

void Song::setSustainThreshold() const
{
	Sustained<true>::s_minLength = m_sustain_cutoff_threshold > 0 ? m_sustain_cutoff_threshold : m_tickrate / 3;
}

uint64_t Song::getHopoThreshold() const noexcept
{
	if (m_hopo_frequency > 0)
		return m_hopo_frequency;
	else if (m_eighthnote_hopo)
		return m_tickrate / 2;
	else if (m_hopofreq_old != UINT16_MAX)
	{
		switch (m_hopofreq_old)
		{
		case 0:
			return m_tickrate / 24;
			break;
		case 1:
			return m_tickrate / 16;
			break;
		case 2:
			return m_tickrate / 12;
			break;
		case 3:
			return m_tickrate / 8;
			break;
		case 4:
			return m_tickrate / 6;
			break;
		default:
			return m_tickrate / 4;
		}
	}
	else
		return m_tickrate / 3;

}

bool Song::load(const std::filesystem::path& chartFile, const ChartType type, bool fullLoad) noexcept
{
	try
	{
		switch (type)
		{
		case ChartType::BCH:
			load_bch(chartFile);
			break;
		case ChartType::CHT:
			load_cht(chartFile, fullLoad);
			break;
		case ChartType::MID:
			load_mid(chartFile);
			break;
		default:
			break;
		}
		checkStartOfTempoMap();
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return false;
	}
	return true;
}

void Song::checkStartOfTempoMap()
{
	if (m_sync.tempoMarkers.isEmpty() || m_sync.tempoMarkers.begin()->key != 0)
		m_sync.tempoMarkers.emplace(m_sync.tempoMarkers.begin(), 0);

	if (m_sync.timeSigs.isEmpty() || m_sync.timeSigs.begin()->key != 0)
		m_sync.timeSigs.emplace(m_sync.timeSigs.begin(), 0, { 4, 2, 24, 8 });
}

bool Song::save(ChartType type) const noexcept
{
	try
	{
		saveIni();
		switch (type)
		{
		case ChartType::BCH:
			save_extended(m_directory / "notes.bch", true);
			break;
		case ChartType::CHT:
			save_extended(m_directory / "notes.cht", false);
			break;
		case ChartType::MID:
			save_mid(m_directory / "notes.mid");
			break;
		default:
			break;
		}
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return false;
	}
	return true;
}

void Song::clear()
{
	m_sync.tempoMarkers.clear();
	m_sync.timeSigs.clear();
	m_events.sections.clear();
	m_events.globals.clear();

	m_noteTracks.lead_5.clear();
	m_noteTracks.lead_6.clear();
	m_noteTracks.bass_5.clear();
	m_noteTracks.bass_6.clear();
	m_noteTracks.rhythm.clear();
	m_noteTracks.coop.clear();
	m_noteTracks.keys.clear();
	m_noteTracks.drums4_pro.clear();
	m_noteTracks.drums5.clear();
	m_noteTracks.vocals.clear();
	m_noteTracks.harmonies.clear();
}

PointerWrapper<Modifiers::Modifier> Song::getModifier(std::string_view name) noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return *iter;
	return {};
}

void Song::validateAudioStreams()
{
	std::tuple<std::filesystem::path, PointerWrapper<std::u32string>, bool> VALIDSTREAMS[] =
	{ 
		{ U"song", 	   m_musicStream, false },
		{ U"guitar",   m_guitarStream, false },
		{ U"rhythm",   m_rhythmStream, false },
		{ U"bass", 	   m_bassStream, false },
		{ U"keys", 	   m_keysStream, false },
		{ U"drums_1",  m_drumStream, false },
		{ U"drums_2",  m_drum2Stream, false },
		{ U"drums_3",  m_drum3Stream, false },
		{ U"drums_4",  m_drum4Stream, false },
		{ U"vocals_1", m_vocalStream, false },
		{ U"vocals_2", m_harmonyStream, false },
		{ U"crowd",	   m_crowdStream, false }
	};
	static const std::filesystem::path AUDIOFORMATS[] = { U".ogg", U".opus", U".mp3", U".wav", U".flac" };

	for (const auto& file : std::filesystem::directory_iterator(m_directory))
	{
		const std::filesystem::path stem = file.path().stem();
		size_t i = 0;
		for (const auto& stream : VALIDSTREAMS)
		{
			if (*std::get<1>(stream) == stem)
				break;
			++i;
		}

		if (i >= std::size(VALIDSTREAMS))
			continue;

		const std::filesystem::path ext = file.path().extension();
		for (const auto& format : AUDIOFORMATS)
		{
			if (format == ext)
			{
				*std::get<1>(VALIDSTREAMS[i]) = file.path().u32string();
				std::get<2>(VALIDSTREAMS[i]) = true;
				break;
			}
		}
	}

	for (const auto& stream : VALIDSTREAMS)
		if (!std::get<2>(stream) && !std::get<1>(stream)->empty() && !std::filesystem::exists(*std::get<1>(stream)))
			std::get<1>(stream)->clear();
}
