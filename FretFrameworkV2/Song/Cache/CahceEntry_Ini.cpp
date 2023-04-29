#include "CacheEntry.h"
#include "Song/Info/Ini.h"

void CacheEntry::readIni(const std::filesystem::path& path, std::filesystem::file_time_type iniTime)
{
	m_modifiers = Ini::ReadSongIniFile(path);
	m_iniModifiedTime = iniTime;
}
