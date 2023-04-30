#include "CacheEntry.h"

using ModifierNode = TxtFileReader::ModifierNode;
const TxtFileReader::ModifierOutline MODIFIER_LIST =
{
	{ "Album",        { "album", ModifierNode::STRING_CHART } },
	{ "Artist",       { "artist", ModifierNode::STRING_CHART } },
	{ "Charter",      { "charter", ModifierNode::STRING_CHART } },
	{ "Difficulty",   { "diff_band", ModifierNode::INT32 } },
	{ "FileVersion",  { "FileVersion", ModifierNode::UINT16 } },
	{ "Genre",        { "genre", ModifierNode::STRING_CHART } },
	{ "Name",         { "name", ModifierNode::STRING_CHART } },
	{ "PreviewEnd",   { "preview_end_time", ModifierNode::FLOAT } },
	{ "PreviewStart", { "preview_start_time", ModifierNode::FLOAT } },
	{ "Year",         { "year", ModifierNode::STRING_CHART } },
};

bool CacheEntry::scan_cht(const std::filesystem::path& path)
{
	ChtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	std::pair<int, bool> versionAndUpdate = scan_header_cht(reader);
	if (versionAndUpdate.first > 1)
		scan(reader);
	else
		scan_cht_V1(reader);
	return versionAndUpdate.second;
}

std::pair<int, bool> CacheEntry::scan_header_cht(ChtFileReader& reader)
{
	static constexpr auto isModifierDefault = [](const Modifiers::Modifier& modifier)
	{
		if (modifier.getName() == "name")             return modifier.getValue<UnicodeString>() == s_DEFAULT_NAME;
		else if (modifier.getName() == "artist")      return modifier.getValue<UnicodeString>() == s_DEFAULT_ARTIST;
		else if (modifier.getName() == "album")       return modifier.getValue<UnicodeString>() == s_DEFAULT_ALBUM;
		else if (modifier.getName() == "genre")       return modifier.getValue<UnicodeString>() == s_DEFAULT_GENRE;
		else if (modifier.getName() == "year")        return modifier.getValue<UnicodeString>() == s_DEFAULT_YEAR;
		else if (modifier.getName() == "charter")     return modifier.getValue<UnicodeString>() == s_DEFAULT_CHARTER;
		else if (modifier.getName() == "diff_band")   return modifier.getValue<int32_t>() == -1;
		return false;
	};

	std::pair<int, bool> versionAndUpdate;
	for (auto& mod : reader.extractModifiers(MODIFIER_LIST))
	{
		if (mod.getName() == "FileVersion")
			versionAndUpdate.first = mod.getValue<uint32_t>();
		else if (auto modifier = getModifier(mod.getName()))
		{
			if (isModifierDefault(*modifier))
			{
				*modifier = std::move(mod);
				versionAndUpdate.second = true;
			}
		}
		else
		{
			m_modifiers.push_back(std::move(mod));
			versionAndUpdate.second = true;
		}
	}
	return versionAndUpdate;
}
