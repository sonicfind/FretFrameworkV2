#include "Song.h"
#include "Ini/Ini.h"

void Song::loadIni()
{
	const std::filesystem::path iniFile(m_directory / U"song.ini");
	if (!std::filesystem::exists(iniFile))
		return;
	
	bool five_lane_drumsSet = false;
	bool hopo_frequencySet = false;
	bool multiplier_noteSet = false;
	bool eighthnote_hopoSet = false;
	bool sustain_thresholdSet = false;
	bool hopofreqSet = false;
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
				m_hopo_frequency = mod.getValue<uint64_t>();
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
				m_sustain_cutoff_threshold = mod.getValue<uint64_t>();
			sustain_thresholdSet = true;
		}
		else if (mod.getName() == "hopofreq")
		{
			if (!hopofreqSet)
				m_hopofreq_old = mod.getValue<uint16_t>();
			hopofreqSet = true;
		}
		else if (!getModifier(mod.getName()))
			m_modifiers.push_back(mod);
	}
}

bool Song::compareToIni()
{
	const std::filesystem::path iniFile(m_directory / U"song.ini");
	if (!std::filesystem::exists(iniFile))
		return false;

	bool five_lane_drumsTested = false;
	bool hopo_frequencyTested = false;
	bool multiplier_noteTested = false;
	bool eighthnote_hopoTested = false;
	bool sustain_thresholdTested = false;
	bool hopofreqTested = false;
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
			if (!hopo_frequencyTested && m_hopo_frequency != mod.getValue<uint64_t>())
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
			if (!sustain_thresholdTested && m_sustain_cutoff_threshold != mod.getValue<uint64_t>())
				return false;
			sustain_thresholdTested = true;
		}
		else if (mod.getName() == "hopofreq")
		{
			if (!hopofreqTested && m_hopofreq_old != mod.getValue<uint16_t>())
				return false;
			hopofreqTested = true;
		}
	}
	return true;
}

void Song::saveIni() const
{
	std::vector<Modifiers::Modifier> modifiers;
	if (!m_name.empty())
		modifiers.push_back({ "name", m_name });
	else
		modifiers.push_back({ "name", std::u32string(U"Unknown Title") });

	if (!m_artist.empty())
		modifiers.push_back({ "artist", m_artist });
	if (!m_album.empty())
		modifiers.push_back({ "album", m_album });
	if (!m_genre.empty())
		modifiers.push_back({ "genre", m_genre });
	if (!m_year.empty())
		modifiers.push_back({ "year", m_year });
	if (!m_charter.empty())
		modifiers.push_back({ "charter", m_charter });
	if (!m_playlist.empty())
		modifiers.push_back({ "playlist", m_playlist });
	modifiers.insert(modifiers.end(), m_modifiers.begin(), m_modifiers.end());

	for (size_t i = 0; i < modifiers.size(); ++i)
	{
		if (modifiers[i].getName() == "five_lane_drums")
		{
			if (m_noteTracks.drums4_pro.isOccupied() && m_noteTracks.drums5.isOccupied())
				modifiers.erase(modifiers.begin() + i);
			else if (m_noteTracks.drums4_pro.isOccupied())
				modifiers[i] = false;
			else if (m_noteTracks.drums5.isOccupied())
				modifiers[i] = true;
			goto Write;
		}
	}

	if (!m_noteTracks.drums4_pro.isOccupied() || !m_noteTracks.drums5.isOccupied())
	{
		if (m_noteTracks.drums4_pro.isOccupied())
			modifiers.push_back({ "five_lane_drums", false });
		else if (m_noteTracks.drums5.isOccupied())
			modifiers.push_back({ "five_lane_drums", true });
	}

Write:
	Ini::WriteSongIniFile(m_directory / U"song.ini", modifiers);
}
