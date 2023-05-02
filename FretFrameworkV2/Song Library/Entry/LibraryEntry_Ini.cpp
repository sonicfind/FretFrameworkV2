#include "LibraryEntry.h"
#include "Ini/Ini.h"

void LibraryEntry::readIni(const std::filesystem::directory_entry& iniFile)
{
	m_modifiers = Ini::ReadSongIniFile(iniFile.path());
	m_iniWriteTime = iniFile.last_write_time();
}

void LibraryEntry::writeIni()
{
	const std::filesystem::path iniFile = m_chartFile.first.parent_path() / "song.ini";
	Ini::WriteSongIniFile(iniFile, m_modifiers);
	m_iniWriteTime = std::filesystem::last_write_time(iniFile);
}

std::u32string LibraryEntry::grabLoadingPhrase() const noexcept
{
	auto mods = Ini::ReadSongIniFile(m_chartFile.first.parent_path() / "song.ini");
	if (mods.empty())
		return {};
	return mods.front().getValue<std::u32string>();
}
