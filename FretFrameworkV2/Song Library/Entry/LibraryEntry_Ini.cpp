#include "LibraryEntry.h"
#include "Ini/Ini.h"

void LibraryEntry::readIni(const std::filesystem::path& path, std::filesystem::file_time_type iniTime)
{
	m_modifiers = Ini::ReadSongIniFile(path);
	m_iniModifiedTime = iniTime;
}

void LibraryEntry::writeIni()
{
	Ini::WriteSongIniFile(m_directory / "song.ini", m_modifiers);
	m_iniModifiedTime = std::filesystem::last_write_time(m_directory / "song.ini");
}
