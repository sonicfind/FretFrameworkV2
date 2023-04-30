#include "LibraryEntry.h"
#include "Ini/Ini.h"

LibraryEntry::LibraryEntry(const std::filesystem::directory_entry& chartFile, const std::filesystem::directory_entry& iniFile)
	: m_chartFile(chartFile), m_modifiers(Ini::ReadSongIniFile(iniFile.path())), m_iniModifiedTime(iniFile.last_write_time()) {}

void LibraryEntry::writeIni()
{
	const std::filesystem::path directory = m_chartFile.path().parent_path();
	Ini::WriteSongIniFile(directory / "song.ini", m_modifiers);
	m_iniModifiedTime = std::filesystem::last_write_time(directory / "song.ini");
}
