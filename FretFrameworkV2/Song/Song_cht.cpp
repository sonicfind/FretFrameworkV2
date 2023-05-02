#include "Song.h"
#include "Serialization/ChtFileWriter.h"

void Song::load_cht(const std::filesystem::path& path, bool isFull)
{
	ChtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	const int16_t version = isFull ? load_header_cht(reader) : load_header_cht_basic(reader);
	if (version > 1)
		traverse(reader);
	else
		traverse_cht_V1(reader);
}

void Song::save_cht(const std::filesystem::path& path)
{
	ChtFileWriter writer(path);
	save(writer);
}

using ModifierNode = TxtFileReader::ModifierNode;
int16_t Song::load_header_cht(ChtFileReader& reader)
{
	static const TxtFileReader::ModifierOutline FULL_MODIFIERS =
	{
		{ "Album",        { "album", ModifierNode::STRING_CHART_NOCASE } },
		{ "Artist",       { "artist", ModifierNode::STRING_CHART_NOCASE } },
		{ "BassStream",   { "BassStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "Charter",      { "charter", ModifierNode::STRING_CHART_NOCASE } },
		{ "CrowdStream",  { "CrowdStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "Difficulty",   { "diff_band", ModifierNode::INT32 } },
		{ "Drum2Stream",  { "Drum2Stream", ModifierNode::STRING_CHART_NOCASE } },
		{ "Drum3Stream",  { "Drum3Stream", ModifierNode::STRING_CHART_NOCASE } },
		{ "Drum4Stream",  { "Drum4Stream", ModifierNode::STRING_CHART_NOCASE } },
		{ "DrumStream",   { "DrumStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "FileVersion",  { "FileVersion", ModifierNode::INT16 } },
		{ "Genre",        { "genre", ModifierNode::STRING_CHART_NOCASE } },
		{ "GuitarStream", { "GuitarStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "KeysStream",   { "KeysStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "MusicStream",  { "MusicStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "Name",         { "name", ModifierNode::STRING_CHART_NOCASE } },
		{ "Offset",       { "delay", ModifierNode::FLOAT } },
		{ "PreviewEnd",   { "preview_end_time", ModifierNode::FLOAT } },
		{ "PreviewStart", { "preview_start_time", ModifierNode::FLOAT } },
		{ "Resolution",   { "Resolution", ModifierNode::UINT16 } },
		{ "RhythmStream", { "RhythmStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "VocalStream",  { "VocalStream", ModifierNode::STRING_CHART_NOCASE } },
		{ "Year",         { "year", ModifierNode::STRING_CHART_NOCASE } },
	};

	int16_t version = 0;
	auto modifiers = reader.extractModifiers(FULL_MODIFIERS);
	for (const auto& modifier : modifiers)
	{
		if (modifier.getName() == "Resolution")
			m_tickrate = modifier.getValue<uint16_t>();
		else if (modifier.getName() == "FileVersion")
			version = modifier.getValue<int16_t>();
		else if (modifier.getName() == "name")
		{
			if (m_name.empty() || m_name == U"Unknown Title")
				m_name = modifier.getValue<UnicodeString>().get();
		}
		else if (modifier.getName() == "artist")
		{
			if (m_artist.empty() || m_artist == U"Unknown Artist")
				m_artist = modifier.getValue<UnicodeString>().get();
		}
		else if (modifier.getName() == "album")
		{
			if (m_album.empty() || m_album == U"Unknown Album")
				m_album = modifier.getValue<UnicodeString>().get();
		}
		else if (modifier.getName() == "genre")
		{
			if (m_genre.empty() || m_genre == U"Unknown Genre")
				m_genre = modifier.getValue<UnicodeString>().get();
		}
		else if (modifier.getName() == "year")
		{
			if (m_year.empty() || m_year == U"Unknown Year")
				m_year = modifier.getValue<UnicodeString>().get();
		}
		else if (modifier.getName() == "charter")
		{
			if (m_charter.empty() || m_charter == U"Unknown Charter")
				m_charter = modifier.getValue<UnicodeString>().get();
		}
		else if (modifier.getName() == "playlist")
		{
			if (m_playlist.empty())
				m_playlist = modifier.getValue<UnicodeString>().get();
		}
	}
	return version;
}

int16_t Song::load_header_cht_basic(ChtFileReader& reader)
{
	static const TxtFileReader::ModifierOutline PARTIAL_MODIFIERS =
	{
		{ "FileVersion", { "FileVersion", ModifierNode::INT16 } },
		{ "Resolution",  { "Resolution" , ModifierNode::UINT16 } },
	};

	std::pair<bool, bool> checked;
	int16_t version = 0;
	auto modifiers = reader.extractModifiers(PARTIAL_MODIFIERS);
	for (const auto& modifier : modifiers)
	{
		if (modifier.getName() == "Resolution")
		{
			if (!checked.first)
			{
				m_tickrate = modifier.getValue<uint16_t>();
				if (checked.second)
					break;
			}
			checked.first = true;
		}
		else if (modifier.getName() == "FileVersion")
		{
			if (!checked.second)
			{
				version = modifier.getValue<int16_t>();
				if (checked.first)
					break;
			}
			checked.second = true;
		}
	}
	return version;
}
