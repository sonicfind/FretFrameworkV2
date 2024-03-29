#include "LibraryEntry.h"
#include <iostream>

const UnicodeString LibraryEntry::s_DEFAULT_NAME{ U"Unknown Title" };
const UnicodeString LibraryEntry::s_DEFAULT_ARTIST{ U"Unknown Artist" };
const UnicodeString LibraryEntry::s_DEFAULT_ALBUM{ U"Unknown Album" };
const UnicodeString LibraryEntry::s_DEFAULT_GENRE{ U"Unknown Genre" };
const UnicodeString LibraryEntry::s_DEFAULT_YEAR{ U"Unknown Year" };
const UnicodeString LibraryEntry::s_DEFAULT_CHARTER{ U"Unknown Charter" };

LibraryEntry::LibraryEntry(const std::pair<std::filesystem::path, ChartType>& chartpath, const std::filesystem::file_time_type& chartLastWrite, const std::filesystem::file_time_type& iniLastWrite)
	: m_chartFile(chartpath), m_chartWriteTime(chartLastWrite), m_iniWriteTime(iniLastWrite) {}

void LibraryEntry::mapStrings(UnicodeWrapper name, UnicodeWrapper artist, UnicodeWrapper album, UnicodeWrapper genre, UnicodeWrapper year, UnicodeWrapper charter, UnicodeWrapper playlist)
{
	m_name = name;
	m_artist = artist;
	m_album = album;
	m_genre = genre;
	m_year = year;
	m_charter = charter;
	m_playlist = playlist;
}

bool LibraryEntry::scan(const LoadedFile& file) noexcept
{
	try
	{
		if (m_chartFile.second == ChartType::CHT)
			scan_cht(file);

		if (!getModifier("name"))
			return false;

		if (m_chartFile.second == ChartType::MID)
			scan_mid(file);
		else if (m_chartFile.second == ChartType::BCH)
			scan_bch(file);

		if (validateForNotes())
			return true;
		return false;
	}
	catch (std::runtime_error err)
	{
		return false;
	}
}

void LibraryEntry::finalize()
{
	if (m_modifiers.empty())
		return;

	reorderModifiers();
	if (m_rewriteIni)
	{
		writeIni();
		m_rewriteIni = false;
	}
	mapModifierVariables();
}

void LibraryEntry::extractSongInfo(BufferedBinaryReader& reader)
{
	ScanValues* const arr[] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.proGuitar_17,
		&m_scanTracks.proGuitar_22,
		&m_scanTracks.proBass_17,
		&m_scanTracks.proBass_22,
		&m_scanTracks.proKeys,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies,
	};

	for (auto track : arr)
		reader.extract(*track);

	reader.extract(m_previewRange);
	reader.extract(m_album_track);
	reader.extract(m_playlist_track);
	reader.extract(m_song_length);
	m_icon = UnicodeString::strToU32(reader.extractString());
	m_source = UnicodeString::strToU32(reader.extractString());

	reader.extract(m_hopo_frequency);
	reader.extract(m_sustain_cutoff_threshold);
	
	reader.extract(m_hopofreq_Old);
	reader.extract(m_eighthnote_hopo);
	reader.extract(m_multiplier_note);
}

void LibraryEntry::serializeFileInfo(BufferedBinaryWriter& writer) const noexcept
{
	writer.writeString(UnicodeString::U32ToStr(m_chartFile.first.parent_path().u32string()));
	writer.writeString(UnicodeString::U32ToStr(m_chartFile.first.filename().u32string()));
	writer.write(m_chartWriteTime.time_since_epoch().count());
	writer.write(m_iniWriteTime.time_since_epoch().count());
}

void LibraryEntry::serializeSongInfo(BufferedBinaryWriter& writer) const noexcept
{
	const ScanValues* const arr[] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.proGuitar_17,
		&m_scanTracks.proGuitar_22,
		&m_scanTracks.proBass_17,
		&m_scanTracks.proBass_22,
		&m_scanTracks.proKeys,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies,
	};

	for (auto track : arr)
		writer.write(*track);

	writer.write(m_previewRange);
	writer.write(m_album_track);
	writer.write(m_playlist_track);
	writer.write(m_song_length);
	writer.writeString(UnicodeString::U32ToStr(m_icon));
	writer.writeString(UnicodeString::U32ToStr(m_source));

	writer.write(m_hopo_frequency);
	writer.write(m_sustain_cutoff_threshold);

	writer.write(m_hopofreq_Old);
	writer.write(m_eighthnote_hopo);
	writer.write(m_multiplier_note);
}

ScanValues LibraryEntry::getScanValue(NoteTrackType track) const noexcept
{
	switch (track)
	{
	case NoteTrackType::Lead:          return m_scanTracks.lead_5;
	case NoteTrackType::Lead_6:        return m_scanTracks.lead_6;
	case NoteTrackType::Bass:          return m_scanTracks.bass_5;
	case NoteTrackType::Bass_6:        return m_scanTracks.bass_6;
	case NoteTrackType::Rhythm:        return m_scanTracks.rhythm;
	case NoteTrackType::Coop:          return m_scanTracks.coop;
	case NoteTrackType::Keys:          return m_scanTracks.keys;
	case NoteTrackType::Drums_4:       return m_scanTracks.drums4;
	case NoteTrackType::Drums_4Pro:    return m_scanTracks.drums4_pro;
	case NoteTrackType::Drums_5:       return m_scanTracks.drums5;
	case NoteTrackType::Vocals:        return m_scanTracks.vocals;
	case NoteTrackType::Harmonies:     return m_scanTracks.harmonies;
	case NoteTrackType::ProGuitar_17:  return m_scanTracks.proGuitar_17;
	case NoteTrackType::ProGuitar_22:  return m_scanTracks.proGuitar_22;
	case NoteTrackType::ProBass_17:  return m_scanTracks.proBass_17;
	case NoteTrackType::ProBass_22:  return m_scanTracks.proBass_22;
	default: return ScanValues();
	}
}

DrumType_Enum LibraryEntry::getDrumType() const noexcept
{
	if (m_scanTracks.drums4.getSubTracks() > 0)
		return DrumType_Enum::FOURLANE_PRO;
	else if (m_scanTracks.drums5.getSubTracks() > 0)
		return DrumType_Enum::FIVELANE;
	return DrumType_Enum::LEGACY;
}

PointerWrapper<const Modifiers::Modifier> LibraryEntry::getModifier(std::string_view name) const noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return *iter;
	return {};
}

PointerWrapper<Modifiers::Modifier> LibraryEntry::getModifier(std::string_view name) noexcept
{
	for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		if (iter->getName() == name)
			return *iter;
	return {};
}

DrumType_Enum LibraryEntry::getDrumTypeFromModifier() const noexcept
{
	if (auto fivelane = getModifier("five_lane_drums"))
		return fivelane->getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;
	return DrumType_Enum::LEGACY;
}

bool LibraryEntry::validateForNotes() const noexcept
{
	const ScanValues* const arr[] =
	{
		&m_scanTracks.lead_5,
		&m_scanTracks.lead_6,
		&m_scanTracks.bass_5,
		&m_scanTracks.bass_6,
		&m_scanTracks.rhythm,
		&m_scanTracks.coop,
		&m_scanTracks.keys,
		&m_scanTracks.drums4_pro,
		&m_scanTracks.drums5,
		&m_scanTracks.proGuitar_17,
		&m_scanTracks.proGuitar_22,
		&m_scanTracks.proBass_17,
		&m_scanTracks.proBass_22,
		&m_scanTracks.proKeys,
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies,
	};

	for (const ScanValues* track : arr)
		if (track->getSubTracks() > 0)
			return true;
	return false;
}

void LibraryEntry::reorderModifiers()
{
	static constexpr std::string_view INI_ORDER[]
	{
		"name", "artist", "album", "genre", "year", "charter", "playlist",
		"album_track", "playlist_track",
		"diff_band",
		"diff_guitar", "diff_guitarghl", "diff_guitar_real", "diff_guitar_real_22",
		"diff_bass", "diff_bassghl", "diff_bass_real", "diff_bass_real_22",
		"diff_rhythm", "diff_guitar_coop",
		"diff_drums", "diff_drums_real", "diff_drums_real_ps",
		"diff_keys", "diff_keys_real", "diff_keys_real_ps",
		"diff_vocals", "diff_vocals_harm",
		"preview_start_time", "preview_end_time",
	};


	std::vector<Modifiers::Modifier> reorder;
	reorder.reserve(m_modifiers.size());
	for (std::string_view str : INI_ORDER)
	{
		for (auto iter = m_modifiers.begin(); iter < m_modifiers.end(); iter++)
		{
			if (iter->getName() == str)
			{
				reorder.push_back(std::move(*iter));
				m_modifiers.erase(iter);
				break;
			}
		}
	}

	for (Modifiers::Modifier& mod : m_modifiers)
		reorder.push_back(std::move(mod));
	m_modifiers = std::move(reorder);
}

void LibraryEntry::mapModifierVariables()
{
	for (size_t i = 0; i < m_modifiers.size(); ++i)
	{
		if (m_modifiers[i].getName() == "loading_phrase")
		{
			m_modifiers.erase(m_modifiers.begin() + i);
			break;
		}
	}

	m_name = m_modifiers.front().getValue<UnicodeString>();
	std::vector<Modifiers::Modifier>::const_iterator iter = m_modifiers.begin() + 1;
	const auto apply = [&](UnicodeWrapper& ptr, std::string_view str, const UnicodeString& def)
	{
		if (iter != m_modifiers.end() && iter->getName() == str)
		{
			ptr = iter->getValue<UnicodeString>();
			++iter;
		}
		else
			ptr = def;
	};
	apply(m_artist, "artist", s_DEFAULT_ARTIST);
	apply(m_album, "album", s_DEFAULT_ALBUM);
	apply(m_genre, "genre", s_DEFAULT_GENRE);
	apply(m_year, "year", s_DEFAULT_YEAR);
	apply(m_charter, "charter", s_DEFAULT_CHARTER);

	if (iter != m_modifiers.end() && iter->getName() == "playlist")
		m_playlist = iter->getValue<UnicodeString>();
	else
	{
		m_directory_playlist = m_chartFile.first.parent_path().parent_path().u32string();
		m_playlist = m_directory_playlist;
	}

	if (auto modifier = getModifier("song_length"))
		m_song_length = modifier->getValue<uint64_t>();

	if (auto modifier = getModifier("preview_start_time"))
		m_previewRange[0] = modifier->getValue<float>();

	if (auto modifier = getModifier("preview_end_time"))
		m_previewRange[1] = modifier->getValue<float>();

	if (auto modifier = getModifier("album_track"))
		m_album_track = modifier->getValue<uint16_t>();

	if (auto modifier = getModifier("playlist_track"))
		m_playlist_track = modifier->getValue<uint16_t>();

	if (auto modifier = getModifier("icon"))
		m_icon = modifier->getValue<std::u32string>();

	if (auto modifier = getModifier("source"))
		m_source = modifier->getValue<std::u32string>();

	if (auto modifier = getModifier("hopo_frequency"))
		m_hopo_frequency = modifier->getValue<uint64_t>();

	if (auto modifier = getModifier("eighthnote_hopo"))
		m_eighthnote_hopo = modifier->getValue<bool>();

	if (auto modifier = getModifier("hopofreq"))
		m_hopofreq_Old = modifier->getValue<uint16_t>();

	if (auto starPower = getModifier("multiplier_note"))
		if (starPower->getValue<uint16_t>() == 103)
			m_multiplier_note = 103;

	if (auto intensity = getModifier("diff_guitar"))
		m_scanTracks.lead_5.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_guitarghl"))
		m_scanTracks.lead_6.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_bass"))
		m_scanTracks.bass_5.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_bassghl"))
		m_scanTracks.bass_6.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_rhythm"))
		m_scanTracks.rhythm.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_guitar_coop"))
		m_scanTracks.coop.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_keys"))
		m_scanTracks.keys.setIntensity(intensity->getValue<int32_t>());

	if (auto modifier = getModifier("diff_drums"))
	{
		const uint32_t intensity = modifier->getValue<int32_t>();
		m_scanTracks.drums4.setIntensity(intensity);
		m_scanTracks.drums4_pro.setIntensity(intensity);
		m_scanTracks.drums5.setIntensity(intensity);
	}

	if (auto intensity = getModifier("diff_drums_real"))
		m_scanTracks.drums4_pro.setIntensity(intensity->getValue<int32_t>());

	m_scanTracks.drums4 = m_scanTracks.drums4_pro;

	if (auto modifier = getModifier("pro_drums"); modifier && !modifier->getValue<bool>())
		m_scanTracks.drums4_pro.reset();

	if (auto intensity = getModifier("diff_guitar_real"))
		m_scanTracks.proGuitar_17.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_guitar_real_22"))
		m_scanTracks.proGuitar_22.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_bass_real"))
		m_scanTracks.proBass_17.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_bass_real_22"))
		m_scanTracks.proBass_22.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_vocals"))
		m_scanTracks.vocals.setIntensity(intensity->getValue<int32_t>());

	if (auto intensity = getModifier("diff_vocals_harm"))
		m_scanTracks.harmonies.setIntensity(intensity->getValue<int32_t>());
}
