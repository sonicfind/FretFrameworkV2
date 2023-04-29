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
