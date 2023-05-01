#include "LibraryEntry.h"
#include <iostream>

const UnicodeString LibraryEntry::s_DEFAULT_NAME{ U"Unknown Title" };
const UnicodeString LibraryEntry::s_DEFAULT_ARTIST{ U"Unknown Artist" };
const UnicodeString LibraryEntry::s_DEFAULT_ALBUM{ U"Unknown Album" };
const UnicodeString LibraryEntry::s_DEFAULT_GENRE{ U"Unknown Genre" };
const UnicodeString LibraryEntry::s_DEFAULT_YEAR{ U"Unknown Year" };
const UnicodeString LibraryEntry::s_DEFAULT_CHARTER{ U"Unknown Charter" };

LibraryEntry::LibraryEntry(const std::filesystem::directory_entry& chartFile) : m_chartFile(chartFile) {}

bool LibraryEntry::scan(const LoadedFile& file, const ChartType type) noexcept
{
	try
	{
		if (type == CHT)
			scan_cht(file);

		if (!getModifier("name"))
			return false;

		if (type == MID)
			scan_mid(file);
		else if (type == BCH)
			scan_bch(file);

		return validateForNotes();
	}
	catch (std::runtime_error err)
	{
		return false;
	}
}

void LibraryEntry::finalize()
{
	reorderModifiers();
	mapModifierVariables();
	if (m_rewriteIni)
	{
		writeIni();
		m_rewriteIni = false;
	}
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

void LibraryEntry::scan(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
			scan_noteTrack(parser);
		else
			parser.skipTrack();
	}
}

void LibraryEntry::scan_noteTrack(CommonChartParser& parser)
{
	BCH_CHT_Scannable* const arr[11] =
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
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies
	};

	const size_t index = parser.geNoteTrackID();
	if (index < std::size(arr))
		arr[parser.geNoteTrackID()]->scan(parser);
	else //BCH only
		parser.skipTrack();
}

bool LibraryEntry::validateForNotes() const noexcept
{
	const ScanTrack* const arr[11] =
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
		&m_scanTracks.vocals,
		&m_scanTracks.harmonies
	};

	for (const ScanTrack* track : arr)
		if (track->m_subTracks > 0)
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
	if (auto playlist = getModifier("playlist"))
		m_playlist = &playlist->getValue<UnicodeString>();
	else
	{
		m_modifiers.reserve(m_modifiers.size() + 1);
		m_modifiers.push_back({ "playlist", UnicodeString(m_chartFile.path().parent_path().parent_path().u32string()) });
		m_playlist = &m_modifiers.back().getValue<UnicodeString>();
	}

	m_name = &m_modifiers.front().getValue<UnicodeString>();
	std::vector<Modifiers::Modifier>::const_iterator iter = m_modifiers.begin() + 1;
	const auto apply = [&](const UnicodeString*& ptr, std::string_view str, const UnicodeString& def)
	{
		if (iter != m_modifiers.end() && iter->getName() == str)
		{
			ptr = &iter->getValue<UnicodeString>();
			++iter;
		}
		else
			ptr = &def;
	};
	apply(m_artist, "artist", s_DEFAULT_ARTIST);
	apply(m_album, "album", s_DEFAULT_ALBUM);
	apply(m_genre, "genre", s_DEFAULT_GENRE);
	apply(m_year, "year", s_DEFAULT_YEAR);
	apply(m_charter, "charter", s_DEFAULT_CHARTER);

	if (auto modifier = getModifier("song_length"))
		m_song_length = modifier->getValue<uint32_t>();

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
		m_hopo_frequency = modifier->getValue<uint32_t>();

	const std::pair<std::string_view, ScanTrack*> intensities[]
	{
		{ "diff_guitar",      &m_scanTracks.lead_5 },
		{ "diff_guitarghl",	  &m_scanTracks.lead_6 },
		{ "diff_bass",		  &m_scanTracks.bass_5 },
		{ "diff_bassghl",     &m_scanTracks.bass_6 },
		{ "diff_rhythm",	  &m_scanTracks.rhythm },
		{ "diff_guitar_coop", &m_scanTracks.coop },
		{ "diff_keys",		  &m_scanTracks.keys },
		{ "diff_drums",		  &m_scanTracks.drums4_pro },
		{ "diff_drums",		  &m_scanTracks.drums5 },
		{ "diff_vocals",	  &m_scanTracks.vocals },
		{ "diff_vocals_harm", &m_scanTracks.harmonies },
	};

	for (int i = 0; i < 11; ++i)
		if (auto modifier = getModifier(intensities[i].first))
			intensities[i].second->m_intensity = modifier->getValue<int32_t>();
}
