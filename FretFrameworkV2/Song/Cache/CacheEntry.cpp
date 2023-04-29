#include "CacheEntry.h"
#include <iostream>

const UnicodeString CacheEntry::s_DEFAULT_NAME{ U"Unknown Title" };
const UnicodeString CacheEntry::s_DEFAULT_ARTIST{ U"Unknown Artist" };
const UnicodeString CacheEntry::s_DEFAULT_ALBUM{ U"Unknown Album" };
const UnicodeString CacheEntry::s_DEFAULT_GENRE{ U"Unknown Genre" };
const UnicodeString CacheEntry::s_DEFAULT_YEAR{ U"Unknown Year" };
const UnicodeString CacheEntry::s_DEFAULT_CHARTER{ U"Unknown Charter" };

const std::filesystem::path CacheEntry::s_EXTS_CHT[2] = { ".cht", ".chart" };
const std::filesystem::path CacheEntry::s_EXTS_MID[2] = { ".mid", ".midi" };
const std::filesystem::path CacheEntry::s_EXT_BCH = ".bch";

CacheEntry::CacheEntry(std::filesystem::file_time_type chartTime) : m_chartModifiedTime(chartTime) {}
bool CacheEntry::scan(const std::filesystem::path& path) noexcept
{
	bool iniChanged = false;
	try
	{
		const std::filesystem::path ext = path.extension();
		if (ext == s_EXTS_CHT[0] || ext == s_EXTS_CHT[1])
			iniChanged = scan_cht(path);
		
		if (getModifier("name") == m_modifiers.end())
			return false;

		if (ext == s_EXTS_MID[0] || ext == s_EXTS_MID[1])
			scan_mid(path);
		else if (ext == s_EXT_BCH)
			scan_bch(path);
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return false;
	}

	m_filename = path.filename();
	m_directory = path.parent_path();
	reorderModifiers();
	if (iniChanged)
		writeIni();
	return true;
}

std::vector<Modifiers::Modifier>::const_iterator CacheEntry::getModifier(std::string_view name) const noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return iter;
	return m_modifiers.end();
}

std::vector<Modifiers::Modifier>::iterator CacheEntry::getModifier(std::string_view name) noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return iter;
	return m_modifiers.end();
}

void CacheEntry::scan(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
			scan_noteTrack(parser);
		else
			parser.skipTrack();
	}
}

void CacheEntry::scan_noteTrack(CommonChartParser& parser)
{
	BCH_CHT_Scannable* const arr[11] =
	{
		&m_noteTracks.lead_5,
		&m_noteTracks.lead_6,
		&m_noteTracks.bass_5,
		&m_noteTracks.bass_6,
		&m_noteTracks.rhythm,
		&m_noteTracks.coop,
		&m_noteTracks.keys,
		&m_noteTracks.drums4_pro,
		&m_noteTracks.drums5,
		&m_noteTracks.vocals,
		&m_noteTracks.harmonies
	};

	const size_t index = parser.geNoteTrackID();
	if (index < std::size(arr))
		arr[parser.geNoteTrackID()]->scan(parser);
	else //BCH only
		parser.skipTrack();
}

void CacheEntry::reorderModifiers()
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

void CacheEntry::mapModifierVariables()
{
	m_name = &m_modifiers.front().getValue<UnicodeString>();

	std::vector<Modifiers::Modifier>::const_iterator iter = m_modifiers.begin() + 1;
	const auto apply = [&](const UnicodeString*& ptr, std::string_view str, const UnicodeString& def)
	{
		if (iter != m_modifiers.end() && iter->getName() == str)
		{
			ptr = &iter->getValue<UnicodeString>();
			++iter;
		}
		else
			ptr = &def;
	};
	apply(m_artist, "artist", s_DEFAULT_ARTIST);
	apply(m_album, "album", s_DEFAULT_ALBUM);
	apply(m_genre, "genre", s_DEFAULT_GENRE);
	apply(m_year, "year", s_DEFAULT_YEAR);
	apply(m_charter, "charter", s_DEFAULT_CHARTER);

	if (iter != m_modifiers.end() && iter->getName() == "playlist")
	{
		m_playlist = &iter->getValue<UnicodeString>();
		++iter;
	}
	else
	{
		m_modifiers.reserve(m_modifiers.size() + 1);
		m_modifiers.push_back({ "playlist", UnicodeString(m_directory.parent_path().u32string()) });
		m_playlist = &m_modifiers.back().getValue<UnicodeString>();
	}

	if (auto iter = getModifier("song_length"); iter != m_modifiers.end())
		m_song_length = iter->getValue<uint32_t>();

	if (auto iter = getModifier("preview_start_time"); iter != m_modifiers.end())
		m_previewRange[0] = iter->getValue<float>();

	if (auto iter = getModifier("preview_end_time"); iter != m_modifiers.end())
		m_previewRange[1] = iter->getValue<float>();

	if (auto iter = getModifier("album_track"); iter != m_modifiers.end())
		m_album_track = iter->getValue<uint16_t>();

	if (auto iter = getModifier("playlist_track"); iter != m_modifiers.end())
		m_playlist_track = iter->getValue<uint16_t>();

	if (auto iter = getModifier("icon"); iter != m_modifiers.end())
		m_icon = iter->getValue<std::u32string>();

	if (auto iter = getModifier("source"); iter != m_modifiers.end())
		m_source = iter->getValue<std::u32string>();

	if (auto iter = getModifier("hopo_frequency"); iter != m_modifiers.end())
		m_hopo_frequency = iter->getValue<uint32_t>();
}
