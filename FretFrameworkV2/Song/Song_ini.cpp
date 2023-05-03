#include "Song.h"
#include "Ini/Ini.h"

void Song::setMetaData()
{
	m_name.clear();
	m_artist.clear();
	m_album.clear();
	m_genre.clear();
	m_year.clear();
	m_charter.clear();
	m_playlist.clear();
	m_musicStream.clear();
	m_guitarStream.clear();
	m_rhythmStream.clear();
	m_bassStream.clear();
	m_keysStream.clear();
	m_drumStream.clear();
	m_drum2Stream.clear();
	m_drum3Stream.clear();
	m_drum4Stream.clear();
	m_vocalStream.clear();
	m_harmonyStream.clear();
	m_crowdStream.clear();
	m_hopo_frequency = 0;
	m_multiplier_note = 116;
	m_sustain_cutoff_threshold = 0;
	m_eighthnote_hopo = false;
	m_baseDrumType = DrumType_Enum::LEGACY;
	m_modifiers.clear();

	const std::filesystem::path iniFile(m_directory / U"song.ini");
	if (!std::filesystem::exists(iniFile))
		return;
	
	bool five_lane_drumsSet = false;
	bool hopo_frequencySet = false;
	bool multiplier_noteSet = false;
	bool eighthnote_hopoSet = false;
	bool sustain_thresholdSet = false;
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
		else if (mod.getName() == "five_lane_drums")
		{
			if (!five_lane_drumsSet && m_baseDrumType == DrumType_Enum::LEGACY)
				m_baseDrumType = mod.getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;
			five_lane_drumsSet = true;
		}
		else if (mod.getName() == "hopo_frequency")
		{
			if (!hopo_frequencySet)
				m_hopo_frequency = mod.getValue<uint32_t>();
			hopo_frequencySet = true;
		}
		else if (mod.getName() == "multiplier_note")
		{
			if (!multiplier_noteSet && mod.getValue<uint16_t>() == 103)
				m_multiplier_note = 103;
			multiplier_noteSet = true;
		}
		else if (mod.getName() == "eighthnote_hopo")
		{
			if (!eighthnote_hopoSet)
				m_eighthnote_hopo = mod.getValue<bool>();
			eighthnote_hopoSet = true;
		}
		else if (mod.getName() == "sustain_cutoff_threshold")
		{
			if (!sustain_thresholdSet)
				m_sustain_cutoff_threshold = mod.getValue<uint32_t>();
			sustain_thresholdSet = true;
		}
		else
			m_modifiers.push_back(mod);
	}
}

bool Song::loadIni()
{
	const std::filesystem::path iniFile(m_directory / U"song.ini");
	if (!std::filesystem::exists(iniFile))
		return false;

	bool five_lane_drumsTested = false;
	bool hopo_frequencyTested = false;
	bool multiplier_noteTested = false;
	bool eighthnote_hopoTested = false;
	bool sustain_thresholdTested = false;
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
		else if (mod.getName() == "five_lane_drums")
		{
			if (!five_lane_drumsTested)
			{
				if (mod.getValue<bool>())
				{
					if (m_baseDrumType == DrumType_Enum::FOURLANE_PRO)
						return false;
				}
				else if (m_baseDrumType == DrumType_Enum::FIVELANE)
					return false;
			}
			five_lane_drumsTested = true;
		}
		else if (mod.getName() == "hopo_frequency")
		{
			if (!hopo_frequencyTested && m_hopo_frequency != mod.getValue<uint32_t>())
				return false;
			hopo_frequencyTested = true;
		}
		else if (mod.getName() == "multiplier_note")
		{
			if (!multiplier_noteTested && m_multiplier_note != mod.getValue<uint16_t>())
				return false;
			multiplier_noteTested = true;
		}
		else if (mod.getName() == "eighthnote_hopo")
		{
			if (!eighthnote_hopoTested && m_eighthnote_hopo != mod.getValue<bool>())
				return false;
			eighthnote_hopoTested = true;
		}
		else if (mod.getName() == "sustain_cutoff_threshold")
		{
			if (!sustain_thresholdTested && m_sustain_cutoff_threshold != mod.getValue<uint32_t>())
				return false;
			sustain_thresholdTested = true;
		}
	}
	return true;
}
