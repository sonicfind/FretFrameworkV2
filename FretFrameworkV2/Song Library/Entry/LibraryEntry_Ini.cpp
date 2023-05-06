#include "LibraryEntry.h"
#include "Ini/Ini.h"

void LibraryEntry::readIni(const std::filesystem::path& iniPath, const std::filesystem::file_time_type& iniLastWrite)
{
	m_modifiers = Ini::ReadSongIniFile(iniPath);
	m_iniWriteTime = iniLastWrite;
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
