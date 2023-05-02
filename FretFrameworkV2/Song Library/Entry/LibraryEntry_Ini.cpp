#include "LibraryEntry.h"
#include "Ini/Ini.h"

void LibraryEntry::readIni(const std::filesystem::directory_entry& iniFile)
{
	m_modifiers = Ini::ReadSongIniFile(iniFile.path());
	m_iniWriteTime = iniFile.last_write_time();
}

void LibraryEntry::writeIni()
{
	const std::filesystem::path directory = m_chartFile.first.parent_path();
	Ini::WriteSongIniFile(directory / "song.ini", m_modifiers);
	m_iniWriteTime = std::filesystem::last_write_time(directory / "song.ini");
}
