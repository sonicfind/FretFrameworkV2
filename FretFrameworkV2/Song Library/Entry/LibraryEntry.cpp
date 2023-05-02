#include "LibraryEntry.h"
#include <iostream>

const UnicodeString LibraryEntry::s_DEFAULT_NAME{ U"Unknown Title" };
const UnicodeString LibraryEntry::s_DEFAULT_ARTIST{ U"Unknown Artist" };
const UnicodeString LibraryEntry::s_DEFAULT_ALBUM{ U"Unknown Album" };
const UnicodeString LibraryEntry::s_DEFAULT_GENRE{ U"Unknown Genre" };
const UnicodeString LibraryEntry::s_DEFAULT_YEAR{ U"Unknown Year" };
const UnicodeString LibraryEntry::s_DEFAULT_CHARTER{ U"Unknown Charter" };

LibraryEntry::LibraryEntry(const std::pair<std::filesystem::path, ChartType>& chartpath, const std::filesystem::file_time_type& chartLastWrite, const std::filesystem::file_time_type& iniLastWrite)
	: m_chartFile(chartpath), m_chartWriteTime(chartLastWrite), m_iniWriteTime(iniLastWrite) {}

void LibraryEntry::mapStrings(UnicodeWrapper name, UnicodeWrapper artist, UnicodeWrapper album, UnicodeWrapper genre, UnicodeWrapper year, UnicodeWrapper charter, UnicodeWrapper playlist)
{
	m_name = name;
	m_artist = artist;
	m_album = album;
	m_genre = genre;
	m_year = year;
	m_charter = charter;
	m_playlist = playlist;
}

bool LibraryEntry::scan(const LoadedFile& file) noexcept
{
	try
	{
		if (m_chartFile.second == ChartType::CHT)
			scan_cht(file);

		if (!getModifier("name"))
			return false;

		if (m_chartFile.second == ChartType::MID)
			scan_mid(file);
		else if (m_chartFile.second == ChartType::BCH)
			scan_bch(file);

		return validateForNotes();
	}
	catch (std::runtime_error err)
	{
		return false;
	}
}

void LibraryEntry::finalize()
{
	if (m_modifiers.empty())
		return;

	reorderModifiers();
	if (m_rewriteIni)
	{
		writeIni();
		m_rewriteIni = false;
	}
	mapModifierVariables();
}

void LibraryEntry::extractSongInfo(BufferedBinaryReader& reader)
{
	ScanTrack* const arr[11] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies
	};

	for (auto track : arr)
	{
		reader.extract(track->m_subTracks);
		reader.extract(track->m_intensity);
	}
	reader.extract(m_previewRange);
	reader.extract(m_album_track);
	reader.extract(m_playlist_track);
	reader.extract(m_song_length);
	reader.extract(m_hopo_frequency);
	m_icon = UnicodeString::strToU32(reader.extractString());
	m_source = UnicodeString::strToU32(reader.extractString());
}

void LibraryEntry::serializeFileInfo(BufferedBinaryWriter& writer) const noexcept
{
	writer.appendString(UnicodeString::U32ToStr(m_chartFile.first.parent_path().u32string()));
	writer.appendString(UnicodeString::U32ToStr(m_chartFile.first.filename().u32string()));
	writer.append(m_chartWriteTime.time_since_epoch().count());
	writer.append(m_iniWriteTime.time_since_epoch().count());
}

void LibraryEntry::serializeSongInfo(BufferedBinaryWriter& writer) const noexcept
{
	const ScanTrack* const arr[11] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies
	};

	for (auto track : arr)
	{
		writer.append(track->m_subTracks);
		writer.append(track->m_intensity);
	}
	writer.append(m_previewRange);
	writer.append(m_album_track);
	writer.append(m_playlist_track);
	writer.append(m_song_length);
	writer.append(m_hopo_frequency);
	writer.appendString(UnicodeString::U32ToStr(m_icon));
	writer.appendString(UnicodeString::U32ToStr(m_source));
}

DrumType_Enum LibraryEntry::getDrumType() const noexcept
{
	if (m_scanTracks.drums4_pro.m_subTracks > 0)
		return DrumType_Enum::FOURLANE_PRO;
	else if (m_scanTracks.drums5.m_subTracks > 0)
		return DrumType_Enum::FIVELANE;
	return DrumType_Enum::LEGACY;
}

PointerWrapper<const Modifiers::Modifier> LibraryEntry::getModifier(std::string_view name) const noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return *iter;
	return {};
}

PointerWrapper<Modifiers::Modifier> LibraryEntry::getModifier(std::string_view name) noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return *iter;
	return {};
}

void LibraryEntry::scan(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
			scan_noteTrack(parser);
		else
			parser.skipTrack();
	}
}

void LibraryEntry::scan_noteTrack(CommonChartParser& parser)
{
	BCH_CHT_Scannable* const arr[11] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies
	};

	const size_t index = parser.geNoteTrackID();
	if (index < std::size(arr))
		arr[parser.geNoteTrackID()]->scan(parser);
	else //BCH only
		parser.skipTrack();
}

bool LibraryEntry::validateForNotes() const noexcept
{
	const ScanTrack* const arr[11] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies
	};

	for (const ScanTrack* track : arr)
		if (track->m_subTracks > 0)
			return true;
	return false;
}

void LibraryEntry::reorderModifiers()
{
	static constexpr std::string_view INI_ORDER[]
	{
		"name", "artist", "album", "genre", "year", "charter", "playlist",
		"album_track", "playlist_track",
		"diff_band",
		"diff_guitar", "diff_guitarghl", "diff_guitar_real", "diff_guitar_real_22",
		"diff_bass", "diff_bassghl", "diff_bass_real", "diff_bass_real_22",
		"diff_rhythm", "diff_guitar_coop",
		"diff_drums", "diff_drums_real", "diff_drums_real_ps",
		"diff_keys", "diff_keys_real", "diff_keys_real_ps",
		"diff_vocals", "diff_vocals_harm",
		"preview_start_time", "preview_end_time",
	};


	std::vector<Modifiers::Modifier> reorder;
	reorder.reserve(m_modifiers.size());
	for (std::string_view str : INI_ORDER)
	{
		for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		{
			if (iter->getName() == str)
			{
				reorder.push_back(std::move(*iter));
				m_modifiers.erase(iter);
				break;
			}
		}
	}

	for (Modifiers::Modifier& mod : m_modifiers)
		reorder.push_back(std::move(mod));
	m_modifiers = std::move(reorder);
}

void LibraryEntry::mapModifierVariables()
{
	m_name = m_modifiers.front().getValue<UnicodeString>();
	std::vector<Modifiers::Modifier>::const_iterator iter = m_modifiers.begin() + 1;
	const auto apply = [&](UnicodeWrapper& ptr, std::string_view str, const UnicodeString& def)
	{
		if (iter != m_modifiers.end() && iter->getName() == str)
		{
			ptr = iter->getValue<UnicodeString>();
			++iter;
		}
		else
			ptr = def;
	};
	apply(m_artist, "artist", s_DEFAULT_ARTIST);
	apply(m_album, "album", s_DEFAULT_ALBUM);
	apply(m_genre, "genre", s_DEFAULT_GENRE);
	apply(m_year, "year", s_DEFAULT_YEAR);
	apply(m_charter, "charter", s_DEFAULT_CHARTER);

	if (iter != m_modifiers.end() && iter->getName() == "playlist")
		m_playlist = iter->getValue<UnicodeString>();
	else
	{
		m_directory_playlist = m_chartFile.first.parent_path().parent_path().u32string();
		m_playlist = m_directory_playlist;
	}

	if (auto modifier = getModifier("song_length"))
		m_song_length = modifier->getValue<uint32_t>();

	if (auto modifier = getModifier("preview_start_time"))
		m_previewRange[0] = modifier->getValue<float>();

	if (auto modifier = getModifier("preview_end_time"))
		m_previewRange[1] = modifier->getValue<float>();

	if (auto modifier = getModifier("album_track"))
		m_album_track = modifier->getValue<uint16_t>();

	if (auto modifier = getModifier("playlist_track"))
		m_playlist_track = modifier->getValue<uint16_t>();

	if (auto modifier = getModifier("icon"))
		m_icon = modifier->getValue<std::u32string>();

	if (auto modifier = getModifier("source"))
		m_source = modifier->getValue<std::u32string>();

	if (auto modifier = getModifier("hopo_frequency"))
		m_hopo_frequency = modifier->getValue<uint32_t>();

	const std::pair<std::string_view, ScanTrack*> intensities[]
	{
		{ "diff_guitar",      &m_scanTracks.lead_5 },
		{ "diff_guitarghl",	  &m_scanTracks.lead_6 },
		{ "diff_bass",		  &m_scanTracks.bass_5 },
		{ "diff_bassghl",     &m_scanTracks.bass_6 },
		{ "diff_rhythm",	  &m_scanTracks.rhythm },
		{ "diff_guitar_coop", &m_scanTracks.coop },
		{ "diff_keys",		  &m_scanTracks.keys },
		{ "diff_drums",		  &m_scanTracks.drums4_pro },
		{ "diff_drums",		  &m_scanTracks.drums5 },
		{ "diff_vocals",	  &m_scanTracks.vocals },
		{ "diff_vocals_harm", &m_scanTracks.harmonies },
	};

	for (int i = 0; i < 11; ++i)
		if (auto modifier = getModifier(intensities[i].first))
			intensities[i].second->m_intensity = modifier->getValue<int32_t>();
}
