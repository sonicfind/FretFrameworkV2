#include "Song.h"
#include "Serialization/ChtFileWriter.h"

void Song::load_cht(const std::filesystem::path& path)
{
	ChtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	if (load_header_cht(reader) > 1)
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
