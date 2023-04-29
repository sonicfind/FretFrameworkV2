#include "Song.h"
#include "Serialization/ChtFileWriter.h"

void Song::load_cht(const std::filesystem::path& path)
{
	ChtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	if (load_songInfo_cht(reader) > 1)
		load(reader);
	else
		load_cht_V1(reader);
}

void Song::save_cht(const std::filesystem::path& path)
{
	ChtFileWriter writer(path);
	save(writer);
}

int Song::load_songInfo_cht(ChtFileReader& reader)
{
	using ModifierNode = TxtFileReader::ModifierNode;
	static const TxtFileReader::ModifierOutline FULL_MODIFIERS =
	{
		{ "Album",        { "album", ModifierNode::STRING_CHART } },
		{ "Artist",       { "artist", ModifierNode::STRING_CHART } },
		{ "Charter",      { "charter", ModifierNode::STRING_CHART } },
		{ "Difficulty",   { "diff_band", ModifierNode::INT32} },
		{ "FileVersion",  { "FileVersion", ModifierNode::UINT16} },
		{ "Genre",        { "genre", ModifierNode::STRING_CHART } },
		{ "Name",         { "name", ModifierNode::STRING_CHART } },
		{ "Offset",       { "delay", ModifierNode::FLOAT} },
		{ "PreviewEnd",   { "preview_end_time", ModifierNode::FLOAT} },
		{ "PreviewStart", { "preview_start_time", ModifierNode::FLOAT} },
		{ "Resolution",   { "Resolution" , ModifierNode::UINT32 } },
		{ "Year",         { "year", ModifierNode::STRING_CHART } },
	};

	static const TxtFileReader::ModifierOutline PARTIAL_MODIFIERS =
	{
		{ "FileVersion", { "FileVersion", ModifierNode::UINT32 } },
		{ "Resolution",  { "Resolution" , ModifierNode::UINT32 } }
	};

	int version = 0;
	auto modifiers = reader.extractModifiers(FULL_MODIFIERS);
	for (const auto& modifier : modifiers)
	{
		if (modifier.getName() == "Resolution")
			m_tickrate = modifier.getValue<uint32_t>();
		else if (modifier.getName() == "FileVersion")
			version = modifier.getValue<uint32_t>();
	}
	return version;
}
