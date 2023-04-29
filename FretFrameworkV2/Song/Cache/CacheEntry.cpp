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
   // m_fileEntry = entry;
   // m_directory = entry.path().parent_path();
	try
	{
		const std::filesystem::path ext = path.extension();
		if (ext == s_EXTS_CHT[0] || ext == s_EXTS_CHT[1])
			scan_cht(path);
		else if (ext == s_EXTS_MID[0] || ext == s_EXTS_MID[1])
			scan_mid(path);
		else if (ext == s_EXT_BCH)
			scan_bch(path);
		else
			throw std::runtime_error(ext.generic_string() + " is not a valid chart type");
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return false;
	}
	return true;
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
