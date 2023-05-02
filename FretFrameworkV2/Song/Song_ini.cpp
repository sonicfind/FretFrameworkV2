#include "Song.h"
#include "Ini/Ini.h"

void Song::setMetaData(const std::filesystem::path& iniFile)
{
	m_name.clear();
	m_artist.clear();
	m_album.clear();
	m_genre.clear();
	m_year.clear();
	m_charter.clear();
	m_playlist.clear();
	m_hopo_frequency = 0;
	m_baseDrumType = DrumType_Enum::LEGACY;

	if (!std::filesystem::exists(iniFile))
		return;
	
	auto modifiers = Ini::ReadSongIniFile(iniFile);
	for (const auto& mod : modifiers)
	{
		if (mod.getName() == "name")
		{
			if (m_name.empty() || m_name == U"Unknown Title")
				m_name = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "artist")
		{
			if (m_artist.empty() || m_artist == U"Unknown Artist")
				m_artist = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "album")
		{
			if (m_album.empty() || m_album == U"Unknown Album")
				m_album = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "genre")
		{
			if (m_genre.empty() || m_genre == U"Unknown Genre")
				m_genre = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "year")
		{
			if (m_year.empty() || m_year == U"Unknown Year")
				m_year = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "charter")
		{
			if (m_charter.empty() || m_charter == U"Unknown Charter")
				m_charter = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "playlist")
		{
			if (m_playlist.empty() || m_playlist == iniFile.parent_path().parent_path())
				m_playlist = mod.getValue<UnicodeString>().get();
		}
		else if (mod.getName() == "hopo_frequency")
		{
			if (m_hopo_frequency == 0)
				m_hopo_frequency = mod.getValue<uint32_t>();
		}
		else if (mod.getName() == "five_lane_drums")
		{
			if (m_baseDrumType == DrumType_Enum::LEGACY)
				m_baseDrumType = mod.getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;
		}
		else if (mod.getName() == "star_power_note")
		{
			if (mod.getValue<uint16_t>() == 103)
				m_midiStarPowerNote = 103;
		}
		else
			m_modifiers.push_back(mod);
	}
}

bool Song::loadIni(const std::filesystem::path& iniFile)
{
	if (!std::filesystem::exists(iniFile))
		return false;

	auto modifiers = Ini::ReadSongIniFile(iniFile);
	for (const auto& mod : modifiers)
	{
		if (mod.getName() == "name")
		{
			if (m_name != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "artist")
		{
			if (m_artist != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "album")
		{
			if (m_album != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "genre")
		{
			if (m_genre != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "year")
		{
			if (m_year != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "charter")
		{
			if (m_charter != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "playlist")
		{
			if (m_playlist != mod.getValue<UnicodeString>().get())
				return false;
		}
		else if (mod.getName() == "hopo_frequency")
		{
			if (m_hopo_frequency != mod.getValue<uint32_t>())
				return false;
		}
		else if (mod.getName() == "five_lane_drums")
		{
			if (mod.getValue<bool>())
			{
				if (m_baseDrumType == DrumType_Enum::FOURLANE_PRO)
					return false;
			}
			else if (m_baseDrumType == DrumType_Enum::FIVELANE)
				return false;
		}
		else if (mod.getName() == "star_power_note")
		{
			if (m_midiStarPowerNote != mod.getValue<uint16_t>())
				return false;
		}
		else
			m_modifiers.push_back(mod);
	}
	return true;
}
